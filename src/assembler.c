#include "nest.h"

#define JOIN_OP(code, arg) ((((u8)(code)) << 8) | (u8)(arg))

typedef struct {
    usize jump_offset;
    usize jump_dst;
    u8 arg_ext;
} JumpRemap;

static Nst_Bytecode *bc_new(usize len, usize obj_len);
static u8 val_size(usize val);
static usize calc_jump_remaps(JumpRemap *remaps, Nst_InstList *ls);
static void translate_ilist(Nst_Bytecode *bc, Nst_InstList *ls,
                            JumpRemap *remap);
static usize add_op(Nst_OpCode op, usize arg, Nst_Span span, Nst_Bytecode *bc,
                    usize op_i);
static bool assemble_func(Nst_FuncPrototype *func, Nst_Bytecode *bc);

static Nst_Bytecode *bc_new(usize len, usize obj_len)
{
    usize tot_size = sizeof(Nst_Bytecode)
                   + len * sizeof(Nst_Op)
                   + len * sizeof(Nst_Span)
                   + obj_len * sizeof(Nst_Obj *);
    void *block = Nst_calloc(1, tot_size, NULL);
    if (block == NULL)
        return NULL;

    Nst_Bytecode *bc = (Nst_Bytecode *)block;
    bc->copy_count = 0;
    bc->len = len;
    bc->obj_len = obj_len;
    bc->bytecode = (Nst_Op *)(bc + 1);
    bc->positions = (Nst_Span *)(bc->bytecode + len);
    bc->objects = (Nst_Obj **)(bc->positions + len);

    return bc;
}

Nst_Bytecode *Nst_bc_copy(Nst_Bytecode *bc)
{
    bc->copy_count++;
    return bc;
}

Nst_Bytecode *Nst_assemble(Nst_InstList *ls)
{
    usize func_count = ls->functions.len;
    JumpRemap *remap = Nst_calloc_c(Nst_ilist_len(ls), JumpRemap, NULL);
    if (remap == NULL)
        return NULL;

    usize bc_len = calc_jump_remaps(remap, ls);

    Nst_Bytecode *bc = bc_new(bc_len, ls->objects.len + ls->functions.len);
    if (bc == NULL) {
        Nst_free(remap);
        return NULL;
    }

    translate_ilist(bc, ls, remap);
    Nst_free(remap);

    bc->obj_len = ls->objects.len;
    for (usize i = 0, n = ls->objects.len; i < n; i++)
        bc->objects[i] = Nst_inc_ref(Nst_ilist_get_obj(ls, i));

    for (usize i = 0; i < func_count; i++) {
        Nst_FuncPrototype *func = Nst_ilist_get_func(ls, i);
        if (!assemble_func(func, bc)) {
            Nst_bc_destroy(bc);
            return NULL;
        }
    }
    return bc;
}

static u8 val_size(usize val)
{
    return (val > 0xffffff) + (val > 0xffff) + (val > 0xff);
}

// populate remaps & calculate the final op count
static usize calc_jump_remaps(JumpRemap *remaps, Nst_InstList *ls)
{
    usize obj_count = ls->objects.len;
    usize ls_len = Nst_ilist_len(ls);
    usize bc_len = 0;

    // add instruction offsets
    for (usize i = 0; i < ls_len; i++) {
        Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
        remaps[i].jump_offset = bc_len - i;
        if (inst->code == Nst_IC_NO_OP)
            continue;
        if (Nst_ic_is_jump(inst->code))
            bc_len += 1;
        else if (inst->code == Nst_IC_MAKE_FUNC) {
            bc_len += val_size((usize)inst->val + obj_count) + 1;
        } else
            bc_len += val_size((usize)inst->val) + 1;
    }

    while (true) {
        isize expand_idx = -1;

        // add jump destinations
        for (usize i = 0; i < ls_len; i++) {
            Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
            if (!Nst_ic_is_jump(inst->code))
                continue;
            usize dst = inst->val + remaps[inst->val].jump_offset;
            remaps[i].jump_dst = dst;
            // give up if a jump needs more OP_ARG_EXTEND
            if (val_size(dst) > remaps[i].arg_ext) {
                expand_idx = i;
                break;
            }
        }

        if (expand_idx == -1)
            break;

        // add the extension and change the offsets and bc_len accordingly

        u8 new_arg_ext = val_size(remaps[expand_idx].jump_dst);
        u8 offset = new_arg_ext - remaps[expand_idx].arg_ext;
        remaps[expand_idx].arg_ext = new_arg_ext;
        bc_len += offset;
        for (usize i = expand_idx + 1; i < ls_len; i++)
            remaps[i].jump_offset += offset;

        // calculate the jump destinations again
    }
    return bc_len;
}

static void translate_ilist(Nst_Bytecode *bc, Nst_InstList *ls,
                            JumpRemap *remaps)
{
    usize op_i = 0;
    usize obj_count = ls->objects.len;
    for (usize i = 0, n = Nst_ilist_len(ls); i < n; i++) {
        Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
        switch (inst->code) {
        case Nst_IC_NO_OP:
            continue;
        case Nst_IC_POP_VAL:
            op_i = add_op(
                Nst_OP_POP_VAL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_FOR_START:
            op_i = add_op(
                Nst_OP_FOR_START, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_FOR_NEXT:
            op_i = add_op(
                Nst_OP_FOR_NEXT, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_RETURN_VAL:
            op_i = add_op(
                Nst_OP_RETURN_VAL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_RETURN_VARS:
            op_i = add_op(
                Nst_OP_RETURN_VARS, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_SET_VAL_LOC:
            op_i = add_op(
                Nst_OP_SET_VAL_LOC, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_SET_CONT_LOC:
            op_i = add_op(
                Nst_OP_SET_CONT_LOC, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_THROW_ERR:
            op_i = add_op(
                Nst_OP_THROW_ERR, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_POP_CATCH:
            op_i = add_op(
                Nst_OP_POP_CATCH, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_SET_VAL:
            op_i = add_op(
                Nst_OP_SET_VAL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_GET_VAL:
            op_i = add_op(
                Nst_OP_GET_VAL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_PUSH_VAL:
            op_i = add_op(
                Nst_OP_PUSH_VAL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_SET_CONT_VAL:
            op_i = add_op(
                Nst_OP_SET_CONT_VAL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_OP_CALL:
            op_i = add_op(
                Nst_OP_CALL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_OP_SEQ_CALL:
            op_i = add_op(
                Nst_OP_SEQ_CALL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_OP_CAST:
            op_i = add_op(
                Nst_OP_CAST, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_OP_RANGE:
            op_i = add_op(
                Nst_OP_RANGE, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_STACK_OP:
            op_i = add_op(
                Nst_OP_STACK, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_LOCAL_OP:
            op_i = add_op(
                Nst_OP_LOCAL, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_OP_IMPORT:
            op_i = add_op(
                Nst_OP_IMPORT, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_OP_EXTRACT:
            op_i = add_op(
                Nst_OP_EXTRACT, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_DEC_INT:
            op_i = add_op(
                Nst_OP_DEC_INT, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_NEW_INT:
            op_i = add_op(
                Nst_OP_NEW_INT, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_DUP:
            op_i = add_op(
                Nst_OP_DUP, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_ROT_2:
            op_i = add_op(
                Nst_OP_ROT_2, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_ROT_3:
            op_i = add_op(
                Nst_OP_ROT_3, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_MAKE_ARR:
            op_i = add_op(
                Nst_OP_MAKE_ARR, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_MAKE_ARR_REP:
            op_i = add_op(
                Nst_OP_MAKE_ARR_REP, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_MAKE_VEC:
            op_i = add_op(
                Nst_OP_MAKE_VEC, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_MAKE_VEC_REP:
            op_i = add_op(
                Nst_OP_MAKE_VEC_REP, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_MAKE_MAP:
            op_i = add_op(
                Nst_OP_MAKE_MAP, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_SAVE_ERROR:
            op_i = add_op(
                Nst_OP_SAVE_ERROR, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_UNPACK_SEQ:
            op_i = add_op(
                Nst_OP_UNPACK_SEQ, (usize)inst->val, inst->span,
                bc, op_i);
            break;
        case Nst_IC_MAKE_FUNC:
            op_i = add_op(
                Nst_OP_MAKE_FUNC, (usize)inst->val + obj_count, inst->span,
                bc, op_i);
            break;
        case Nst_IC_JUMP:
            op_i = add_op(
                Nst_OP_JUMP, remaps[i].jump_dst, inst->span,
                bc, op_i);
            break;
        case Nst_IC_JUMPIF_T:
            op_i = add_op(
                Nst_OP_JUMPIF_T, remaps[i].jump_dst, inst->span,
                bc, op_i);
            break;
        case Nst_IC_JUMPIF_F:
            op_i = add_op(
                Nst_OP_JUMPIF_F, remaps[i].jump_dst, inst->span,
                bc, op_i);
            break;
        case Nst_IC_JUMPIF_ZERO:
            op_i = add_op(
                Nst_OP_JUMPIF_ZERO, remaps[i].jump_dst, inst->span,
                bc, op_i);
            break;
        case Nst_IC_JUMPIF_IEND:
            op_i = add_op(
                Nst_OP_JUMPIF_IEND, remaps[i].jump_dst, inst->span,
                bc, op_i);
            break;
        case Nst_IC_PUSH_CATCH:
            op_i = add_op(
                Nst_OP_PUSH_CATCH, remaps[i].jump_dst, inst->span,
                bc, op_i);
            break;
        default:
            Nst_assert_c(false);
            break;
        }
    }
}

static usize add_op(Nst_OpCode op, usize arg, Nst_Span span, Nst_Bytecode *bc,
                    usize op_i)
{
    if (arg > 0xffffff) {
        bc->positions[op_i] = span;
        bc->bytecode[op_i++] = JOIN_OP(Nst_OP_EXTEND_ARG, arg >> 24);
    }
    if (arg > 0xffff) {
        bc->positions[op_i] = span;
        bc->bytecode[op_i++] = JOIN_OP(Nst_OP_EXTEND_ARG, arg >> 16);
    }
    if (arg > 0xff) {
        bc->positions[op_i] = span;
        bc->bytecode[op_i++] = JOIN_OP(Nst_OP_EXTEND_ARG, arg >> 8);
    }

    bc->positions[op_i] = span;
    bc->bytecode[op_i++] = JOIN_OP(op, arg);
    return op_i;
}

static bool assemble_func(Nst_FuncPrototype *func, Nst_Bytecode *bc)
{
    Nst_Bytecode *func_bc = Nst_assemble(&func->ilist);
    if (func_bc == NULL)
        return false;
    Nst_Obj *func_obj = _Nst_func_new(func->arg_names, func->arg_num, func_bc);
    if (func_obj == NULL) {
        Nst_bc_destroy(func_bc);
        return false;
    }
    bc->objects[bc->obj_len++] = func_obj;
    return true;
}

void Nst_bc_destroy(Nst_Bytecode *bc)
{
    if (bc == NULL)
        return;
    if (bc->copy_count > 0) {
        bc->copy_count--;
        return;
    }
    for (usize i = 0, n = bc->obj_len; i < n; i++)
        Nst_ndec_ref(bc->objects[i]);
    Nst_free(bc);
}

static void bc_print(Nst_Bytecode *bc, usize indent)
{
    for (usize i = 0; i < indent; i++)
        Nst_print("    ");
    Nst_println("Bytecode:");

    int idx_width = 1;
    usize len = bc->len;
    while (len > 10) {
        len /= 10;
        idx_width++;
    }

    u64 arg = 0;
    bool extend_arg = false;
    for (usize i = 0, n = bc->len; i < n; i++) {
        for (usize j = 0; j < indent; j++)
            Nst_print("    ");
        Nst_printf("%*zi  ", idx_width, i);
        Nst_Op op = bc->bytecode[i];
        switch (Nst_OP_CODE(op)) {
        case Nst_OP_POP_VAL:      Nst_print("pop    "); break;
        case Nst_OP_FOR_START:    Nst_print("istart "); break;
        case Nst_OP_FOR_NEXT:     Nst_print("inext  "); break;
        case Nst_OP_RETURN_VAL:   Nst_print("ret    "); break;
        case Nst_OP_RETURN_VARS:  Nst_print("retvar "); break;
        case Nst_OP_SET_VAL_LOC:  Nst_print("setpop "); break;
        case Nst_OP_SET_CONT_LOC: Nst_print("setcpop"); break;
        case Nst_OP_THROW_ERR:    Nst_print("throw  "); break;
        case Nst_OP_POP_CATCH:    Nst_print("poptry "); break;
        case Nst_OP_SET_VAL:      Nst_print("set    "); break;
        case Nst_OP_GET_VAL:      Nst_print("get    "); break;
        case Nst_OP_PUSH_VAL:     Nst_print("push   "); break;
        case Nst_OP_SET_CONT_VAL: Nst_print("setc   "); break;
        case Nst_OP_CALL:         Nst_print("call   "); break;
        case Nst_OP_SEQ_CALL:     Nst_print("callseq"); break;
        case Nst_OP_CAST:         Nst_print("cast   "); break;
        case Nst_OP_RANGE:        Nst_print("range  "); break;
        case Nst_OP_STACK:        Nst_print("binop  "); break;
        case Nst_OP_LOCAL:        Nst_print("uniop  "); break;
        case Nst_OP_IMPORT:       Nst_print("import "); break;
        case Nst_OP_EXTRACT:      Nst_print("extract"); break;
        case Nst_OP_DEC_INT:      Nst_print("dec    "); break;
        case Nst_OP_NEW_INT:      Nst_print("dupint "); break;
        case Nst_OP_DUP:          Nst_print("dup    "); break;
        case Nst_OP_ROT_2:        Nst_print("rot2   "); break;
        case Nst_OP_ROT_3:        Nst_print("rot3   "); break;
        case Nst_OP_MAKE_ARR:     Nst_print("mkarr  "); break;
        case Nst_OP_MAKE_ARR_REP: Nst_print("fillarr"); break;
        case Nst_OP_MAKE_VEC:     Nst_print("mkvec  "); break;
        case Nst_OP_MAKE_VEC_REP: Nst_print("fillvec"); break;
        case Nst_OP_MAKE_MAP:     Nst_print("mkmap  "); break;
        case Nst_OP_MAKE_FUNC:    Nst_print("mkfunc "); break;
        case Nst_OP_SAVE_ERROR:   Nst_print("geterr "); break;
        case Nst_OP_UNPACK_SEQ:   Nst_print("unpack "); break;
        case Nst_OP_EXTEND_ARG:   Nst_print("extend "); break;
        case Nst_OP_JUMP:         Nst_print("jmp    "); break;
        case Nst_OP_JUMPIF_T:     Nst_print("jmptrue"); break;
        case Nst_OP_JUMPIF_F:     Nst_print("jmpflse"); break;
        case Nst_OP_JUMPIF_ZERO:  Nst_print("jmpzero"); break;
        case Nst_OP_JUMPIF_IEND:  Nst_print("jmpiend"); break;
        case Nst_OP_PUSH_CATCH:   Nst_print("pushtry"); break;
        default: Nst_assert_c(false);
        }

        if (extend_arg)
            arg = arg << 8 | Nst_OP_ARG(op);
        else
            arg = Nst_OP_ARG(op);
        extend_arg = Nst_OP_CODE(op) == Nst_OP_EXTEND_ARG;

        Nst_printf(" %3" PRIu8, Nst_OP_ARG(op));

        if (arg != Nst_OP_ARG(op))
            Nst_printf(" (extended: %3" PRIu64 ")", arg);
        if (Nst_OP_CODE(op) == Nst_OP_STACK
            || Nst_OP_CODE(op) == Nst_OP_LOCAL)
        {
            Nst_print(" [");

            switch (arg) {
            case Nst_TT_ADD:      Nst_print("+");  break;
            case Nst_TT_SUB:      Nst_print("-");  break;
            case Nst_TT_MUL:      Nst_print("*");  break;
            case Nst_TT_DIV:      Nst_print("/");  break;
            case Nst_TT_POW:      Nst_print("^");  break;
            case Nst_TT_MOD:      Nst_print("%");  break;
            case Nst_TT_B_AND:    Nst_print("&");  break;
            case Nst_TT_B_OR:     Nst_print("|");  break;
            case Nst_TT_LEN:      Nst_print("$");  break;
            case Nst_TT_L_NOT:    Nst_print("!");  break;
            case Nst_TT_B_NOT:    Nst_print("~");  break;
            case Nst_TT_GT:       Nst_print(">");  break;
            case Nst_TT_LT:       Nst_print("<");  break;
            case Nst_TT_B_XOR:    Nst_print("^^"); break;
            case Nst_TT_LSHIFT:   Nst_print("<<"); break;
            case Nst_TT_RSHIFT:   Nst_print(">>"); break;
            case Nst_TT_CONCAT:   Nst_print("><"); break;
            case Nst_TT_L_AND:    Nst_print("&&"); break;
            case Nst_TT_L_OR:     Nst_print("||"); break;
            case Nst_TT_L_XOR:    Nst_print("&|"); break;
            case Nst_TT_EQ:       Nst_print("=="); break;
            case Nst_TT_NEQ:      Nst_print("!="); break;
            case Nst_TT_GTE:      Nst_print(">="); break;
            case Nst_TT_LTE:      Nst_print("<="); break;
            case Nst_TT_NEG:      Nst_print("-:"); break;
            case Nst_TT_STDOUT:   Nst_print(">>>");break;
            case Nst_TT_STDIN:    Nst_print("<<<");break;
            case Nst_TT_TYPEOF:   Nst_print("?::");break;
            case Nst_TT_CONTAINS: Nst_print("<.>");break;
            default: Nst_assert_c(false);
            }
            Nst_print("]");
        } else if (Nst_OP_CODE(op) == Nst_OP_PUSH_VAL
                   || Nst_OP_CODE(op) == Nst_OP_GET_VAL
                   || Nst_OP_CODE(op) == Nst_OP_SET_VAL
                   || Nst_OP_CODE(op) == Nst_OP_SET_VAL_LOC)
        {
            Nst_Obj *obj = bc->objects[arg];
            Nst_printf(" [(%s) ", Nst_type_name(obj->type).value);

            Nst_Obj *s = Nst_obj_to_repr_str(obj);
            if (Nst_error_occurred()) {
                Nst_error_clear();
            } else {
                Nst_fwrite(Nst_str_value(s), Nst_str_len(s), NULL, Nst_io.out);
                Nst_dec_ref(s);
            }
            Nst_print("]");
        }
        Nst_println("");
    }
    Nst_println("");
    for (usize i = 0; i < indent; i++)
        Nst_print("    ");
    Nst_println("Objects:");
    usize width = 1;
    usize obj_len = bc->obj_len;
    while (obj_len > 10) {
        width++;
        obj_len /= 10;
    }

    for (usize i = 0, n = bc->obj_len; i < n; i++) {
        for (usize j = 0; j < indent; j++)
            Nst_print("    ");

        Nst_Obj *obj = bc->objects[i];
        Nst_printf(
            "%*zi  (%s) ",
            (int)width, i,
            Nst_type_name(obj->type).value);

        Nst_Obj *s = Nst_obj_to_repr_str(obj);
        if (Nst_error_occurred()) {
            Nst_error_clear();
        } else {
            Nst_fwrite(Nst_str_value(s), Nst_str_len(s), NULL, Nst_io.out);
            Nst_dec_ref(s);
        }

        Nst_println("");
        if (obj->type == Nst_t.Func)
            bc_print(Nst_func_nest_body(obj), indent + 1);
    }
}

void Nst_bc_print(Nst_Bytecode *bc)
{
    bc_print(bc, 0);
}
