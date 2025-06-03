#include <string.h>
#include "nest.h"

bool Nst_ic_is_jump(Nst_InstCode code)
{
    return code >= Nst_IC_JUMP && code <= Nst_IC_PUSH_CATCH;
}

bool Nst_ilist_init(Nst_InstList *list)
{
    return Nst_da_init(&list->instructions, sizeof(Nst_Inst), 16)
        && Nst_pa_init(&list->objects, 4)
        && Nst_da_init(&list->functions, sizeof(Nst_FuncPrototype), 0);
}

void Nst_ilist_destroy(Nst_InstList *list)
{
    Nst_da_clear(&list->instructions, NULL);
    Nst_pa_clear(&list->objects, (Nst_Destructor)Nst_dec_ref);
    Nst_da_clear(&list->functions, (Nst_Destructor)Nst_fprototype_destroy);
}

bool Nst_ilist_add(Nst_InstList *list, Nst_InstCode code, Nst_Span span)
{
    Nst_Inst inst = { .code = code, .span = span, .val = 0 };
    return Nst_da_append(&list->instructions, &inst);
}

bool Nst_ilist_add_ex(Nst_InstList *list, Nst_InstCode code, i64 val,
                      Nst_Span span)
{
    Nst_Inst inst = { .code = code, .span = span, .val = val };
    return Nst_da_append(&list->instructions, &inst);
}

isize Nst_ilist_add_obj(Nst_InstList *list, Nst_ObjRef *obj)
{
    if (!Nst_pa_append(&list->objects, obj))
        return -1;
    return (isize)(list->objects.len - 1);
}

isize Nst_ilist_add_func(Nst_InstList *list, Nst_FuncPrototype *fp)
{
    if (!Nst_da_append(&list->functions, fp))
        return -1;
    return (isize)(list->functions.len - 1);
}

Nst_Inst *Nst_ilist_get_inst(Nst_InstList *list, usize idx)
{
    return (Nst_Inst *)Nst_da_get(&list->instructions, idx);
}

Nst_Obj *Nst_ilist_get_inst_obj(Nst_InstList *list, usize idx)
{
    Nst_Inst *inst = Nst_ilist_get_inst(list, idx);
    return NstOBJ(Nst_pa_get(&list->objects, (usize)inst->val));
}

Nst_FuncPrototype *Nst_ilist_get_inst_func(Nst_InstList *list, usize idx)
{
    Nst_Inst *inst = Nst_ilist_get_inst(list, idx);
    return (Nst_FuncPrototype *)Nst_da_get(
        &list->functions,
        (usize)inst->val);
}

Nst_Obj *Nst_ilist_get_obj(Nst_InstList *list, usize idx)
{
    return NstOBJ(Nst_pa_get(&list->objects, idx));
}

Nst_FuncPrototype *Nst_ilist_get_func(Nst_InstList *list, usize idx)
{
    return (Nst_FuncPrototype *)Nst_da_get(&list->functions, idx);
}

void Nst_ilist_set(Nst_InstList *list, usize idx, Nst_InstCode code)
{
    Nst_Inst *inst = Nst_ilist_get_inst(list, idx);
    if (inst != NULL) {
        inst->code = code;
        inst->val = 0;
    }
}

void Nst_ilist_set_ex(Nst_InstList *list, usize idx, Nst_InstCode code,
                      i64 val)
{
    Nst_Inst *inst = Nst_ilist_get_inst(list, idx);
    if (inst != NULL) {
        inst->code = code;
        inst->val = val;
    }
}

usize Nst_ilist_len(Nst_InstList *list)
{
    return list->instructions.len;
}

bool Nst_fprototype_init(Nst_FuncPrototype *fp, Nst_InstList ls, usize arg_num)
{
    fp->ilist = ls;
    fp->arg_num = arg_num;
    if (arg_num == 0) {
        fp->arg_names = NULL;
        return true;
    } else {
        fp->arg_names = Nst_calloc(arg_num, sizeof(Nst_Obj *), NULL);
        return fp->arg_names != NULL;
    }
}

void Nst_fprototype_destroy(Nst_FuncPrototype *fp)
{
    if (fp->arg_names != NULL) {
        for (usize i = 0, n = fp->arg_num; i < n; i++)
            Nst_ndec_ref(fp->arg_names[i]);
        Nst_free(fp->arg_names);
    }
    Nst_ilist_destroy(&fp->ilist);
}

static void print_bytecode(Nst_InstList *ls, i32 indent)
{
    usize tot_size = ls->instructions.len;
    int idx_width = 1;
    int max_col = 0;
    int max_row = 0;
    int col_width = 1;
    int row_width = 1;

    for (usize i = 0, n = tot_size; i < n; i++) {
        Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
        if (inst->span.start_col > max_col)
            max_col = inst->span.start_col;
        if (inst->span.start_line > max_row)
            max_row = inst->span.start_line;
    }

    while (tot_size >= 10) {
        tot_size /= 10;
        idx_width++;
    }
    while (max_col >= 10) {
        max_col /= 10;
        col_width++;
    }
    while (max_row >= 10) {
        max_row /= 10;
        row_width++;
    }

    idx_width = idx_width < 3 ? 3 : idx_width;
    col_width = col_width < 3 ? 3 : col_width;
    row_width = row_width < 3 ? 3 : row_width;

    for (i32 i = 0; i < indent; i++)
        Nst_print("    ");
    for (i32 i = 3; i < idx_width; i++)
        Nst_print(" ");
    Nst_print(" Idx |");
    for (i32 i = 3; i < row_width; i++)
        Nst_print(" ");
    Nst_print("   Pos   ");
    for (i32 i = 3; i < col_width; i++)
        Nst_print(" ");
    Nst_print("|  Instruction  | Value\n");

    for (usize i = 0, n = ls->instructions.len; i < n; i++) {
        Nst_Inst *inst = Nst_ilist_get_inst(ls, i);
        Nst_InstCode code = inst->code;

        for (i32 j = 0; j < indent; j++)
            Nst_print("    ");
        Nst_printf(
            " %*zi | %*" PRIi32 ":%-*" PRIi32 " | ",
            idx_width, i,
            row_width,
            inst->span.start_line + 1,
            col_width,
            inst->span.start_col + 1);

        switch (code) {
        case Nst_IC_NO_OP:         Nst_print("NO_OP        "); break;
        case Nst_IC_POP_VAL:       Nst_print("POP_VAL      "); break;
        case Nst_IC_FOR_START:     Nst_print("FOR_START    "); break;
        case Nst_IC_RETURN_VAL:    Nst_print("RETURN_VAL   "); break;
        case Nst_IC_RETURN_VARS:   Nst_print("RETURN_VARS  "); break;
        case Nst_IC_SET_VAL_LOC:   Nst_print("SET_VAL_LOC  "); break;
        case Nst_IC_SET_CONT_LOC:  Nst_print("SET_CONT_LOC "); break;
        case Nst_IC_JUMP:          Nst_print("JUMP         "); break;
        case Nst_IC_JUMPIF_T:      Nst_print("JUMPIF_T     "); break;
        case Nst_IC_JUMPIF_F:      Nst_print("JUMPIF_F     "); break;
        case Nst_IC_JUMPIF_ZERO:   Nst_print("JUMPIF_ZERO  "); break;
        case Nst_IC_THROW_ERR:     Nst_print("THROW_ERR    "); break;
        case Nst_IC_PUSH_CATCH:    Nst_print("PUSH_CATCH   "); break;
        case Nst_IC_POP_CATCH:     Nst_print("POP_CATCH    "); break;
        case Nst_IC_SET_VAL:       Nst_print("SET_VAL      "); break;
        case Nst_IC_GET_VAL:       Nst_print("GET_VAL      "); break;
        case Nst_IC_PUSH_VAL:      Nst_print("PUSH_VAL     "); break;
        case Nst_IC_SET_CONT_VAL:  Nst_print("SET_CONT_VAL "); break;
        case Nst_IC_OP_CALL:       Nst_print("OP_CALL      "); break;
        case Nst_IC_OP_SEQ_CALL:   Nst_print("OP_SEQ_CALL  "); break;
        case Nst_IC_OP_CAST:       Nst_print("OP_CAST      "); break;
        case Nst_IC_OP_RANGE:      Nst_print("OP_RANGE     "); break;
        case Nst_IC_STACK_OP:      Nst_print("STACK_OP     "); break;
        case Nst_IC_LOCAL_OP:      Nst_print("LOCAL_OP     "); break;
        case Nst_IC_OP_IMPORT:     Nst_print("OP_IMPORT    "); break;
        case Nst_IC_OP_EXTRACT:    Nst_print("OP_EXTRACT   "); break;
        case Nst_IC_DEC_INT:       Nst_print("DEC_INT      "); break;
        case Nst_IC_NEW_INT:       Nst_print("NEW_INT      "); break;
        case Nst_IC_DUP:           Nst_print("DUP          "); break;
        case Nst_IC_ROT_2:         Nst_print("ROT_2        "); break;
        case Nst_IC_ROT_3:         Nst_print("ROT_3        "); break;
        case Nst_IC_MAKE_ARR:      Nst_print("MAKE_ARR     "); break;
        case Nst_IC_MAKE_ARR_REP:  Nst_print("MAKE_ARR_REP "); break;
        case Nst_IC_MAKE_VEC:      Nst_print("MAKE_VEC     "); break;
        case Nst_IC_MAKE_VEC_REP:  Nst_print("MAKE_VEC_REP "); break;
        case Nst_IC_MAKE_MAP:      Nst_print("MAKE_MAP     "); break;
        case Nst_IC_MAKE_FUNC:     Nst_print("MAKE_FUNC    "); break;
        case Nst_IC_JUMPIF_IEND:   Nst_print("FOR_IS_DONE  "); break;
        case Nst_IC_FOR_NEXT:      Nst_print("FOR_NEXT     "); break;
        case Nst_IC_SAVE_ERROR:    Nst_print("SAVE_ERROR   "); break;
        case Nst_IC_UNPACK_SEQ:    Nst_print("UNPACK_SEQ   "); break;
        default: Nst_assert(false);
        }

        Nst_print(" |");

        if (inst->code == Nst_IC_NO_OP) {
            Nst_print("\n");
            continue;
        }

        if (inst->val != 0 || Nst_ic_is_jump(code) || code == Nst_IC_LOCAL_OP
            || code == Nst_IC_STACK_OP || code == Nst_IC_MAKE_ARR
            || code == Nst_IC_MAKE_VEC || code == Nst_IC_MAKE_MAP
            || code == Nst_IC_MAKE_FUNC || code == Nst_IC_PUSH_VAL
            || code == Nst_IC_GET_VAL || code == Nst_IC_SET_VAL
            || code == Nst_IC_SET_VAL_LOC)
        {
            Nst_printf(" %*" PRIi64, idx_width, inst->val);
        } else {
            Nst_println("");
            continue;
        }

        if (code == Nst_IC_PUSH_VAL || code == Nst_IC_GET_VAL
            || code == Nst_IC_SET_VAL || code == Nst_IC_SET_VAL_LOC)
        {
            Nst_Obj *obj = Nst_ilist_get_inst_obj(ls, i);
            Nst_Obj *s = Nst_obj_to_repr_str(obj);
            if (Nst_error_occurred()) {
                Nst_error_clear();
                Nst_println("");
                continue;
            }
            Nst_printf(" [(%s) ", Nst_type_name(obj->type).value);
            Nst_fwrite(Nst_str_value(s), Nst_str_len(s), NULL, Nst_io.out);
            Nst_dec_ref(s);
            Nst_print("]");
        } else if (code == Nst_IC_MAKE_FUNC) {
            Nst_print("\n\n");
            for (i32 j = 0; j < indent + 1; j++)
                Nst_print("    ");
            Nst_println("Function instructions:");
            Nst_FuncPrototype *func = Nst_ilist_get_inst_func(ls, i);
            print_bytecode(&func->ilist, indent + 1);
        } else if (code == Nst_IC_STACK_OP || code == Nst_IC_LOCAL_OP) {
            Nst_print(" [");

            switch (inst->val) {
            case Nst_TT_ADD:     Nst_print("+");  break;
            case Nst_TT_SUB:     Nst_print("-");  break;
            case Nst_TT_MUL:     Nst_print("*");  break;
            case Nst_TT_DIV:     Nst_print("/");  break;
            case Nst_TT_POW:     Nst_print("^");  break;
            case Nst_TT_MOD:     Nst_print("%");  break;
            case Nst_TT_B_AND:   Nst_print("&");  break;
            case Nst_TT_B_OR:    Nst_print("|");  break;
            case Nst_TT_LEN:     Nst_print("$");  break;
            case Nst_TT_L_NOT:   Nst_print("!");  break;
            case Nst_TT_B_NOT:   Nst_print("~");  break;
            case Nst_TT_GT:      Nst_print(">");  break;
            case Nst_TT_LT:      Nst_print("<");  break;
            case Nst_TT_B_XOR:   Nst_print("^^"); break;
            case Nst_TT_LSHIFT:  Nst_print("<<"); break;
            case Nst_TT_RSHIFT:  Nst_print(">>"); break;
            case Nst_TT_CONCAT:  Nst_print("><"); break;
            case Nst_TT_L_AND:   Nst_print("&&"); break;
            case Nst_TT_L_OR:    Nst_print("||"); break;
            case Nst_TT_L_XOR:   Nst_print("&|"); break;
            case Nst_TT_EQ:      Nst_print("=="); break;
            case Nst_TT_NEQ:     Nst_print("!="); break;
            case Nst_TT_GTE:     Nst_print(">="); break;
            case Nst_TT_LTE:     Nst_print("<="); break;
            case Nst_TT_NEG:     Nst_print("-:"); break;
            case Nst_TT_STDOUT:  Nst_print(">>>");break;
            case Nst_TT_STDIN:   Nst_print("<<<");break;
            case Nst_TT_TYPEOF:  Nst_print("?::");break;
            case Nst_TT_CONTAINS:Nst_print("<.>");break;
            default: Nst_assert(false);
            }
            Nst_print("]");
        }
        Nst_println("");
    }
}

void Nst_ilist_print(Nst_InstList *ls)
{
    print_bytecode(ls, 0);
}
