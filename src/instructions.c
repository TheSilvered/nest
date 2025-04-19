#include <string.h>
#include "nest.h"

bool Nst_ic_is_jump(Nst_InstCode code)
{
    return code >= Nst_IC_JUMP && code <= Nst_IC_PUSH_CATCH;
}

bool Nst_ilist_init(Nst_InstList *list)
{
    return Nst_da_init(&list->instructions, sizeof(Nst_Inst), 16)
        && Nst_da_init(&list->objects, sizeof(Nst_Obj *), 4)
        && Nst_da_init(&list->functions, sizeof(Nst_FuncPrototype), 0);
}

void Nst_ilist_destroy(Nst_InstList *list)
{
    Nst_da_clear(&list->instructions, NULL);
    Nst_da_clear_p(&list->objects, (Nst_Destructor)Nst_dec_ref);
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
    if (!Nst_da_append(&list->objects, &obj))
        return -1;
    return (isize)(list->objects.len - 1);
}

isize Nst_ilist_add_func(Nst_InstList *list, Nst_FuncPrototype *fp)
{
    if (!Nst_da_append(&list->functions, &fp))
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
    return NstOBJ(Nst_da_get_p(&list->objects, (usize)inst->val));
}

Nst_FuncPrototype *Nst_ilist_get_inst_func(Nst_InstList *list, usize idx)
{
    Nst_Inst *inst = Nst_ilist_get_inst(list, idx);
    return (Nst_FuncPrototype *)Nst_da_get_p(
        &list->functions,
        (usize)inst->val);
}

Nst_Obj *Nst_ilist_get_obj(Nst_InstList *list, usize idx)
{
    return NstOBJ(Nst_da_get_p(&list->objects, idx));
}

Nst_FuncPrototype *Nst_ilist_get_func(Nst_InstList *list, usize idx)
{
    return (Nst_FuncPrototype *)Nst_da_get_p(&list->functions, idx);
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
    fp->arg_names = Nst_calloc(arg_num, sizeof(Nst_Obj *), NULL);
    return fp->arg_names != NULL;
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

#define PRINT(str, size) Nst_fwrite((u8 *)(str), (size), NULL, Nst_io.out)

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
        PRINT("    ", 4);
    for (i32 i = 3; i < idx_width; i++)
        PRINT(" ", 1);
    PRINT(" Idx |", 6);
    for (i32 i = 3; i < row_width; i++)
        PRINT(" ", 1);
    PRINT("   Pos   ", 9);
    for (i32 i = 3; i < col_width; i++)
        PRINT(" ", 1);
    PRINT("|  Instruction  | Value\n", 24);

    for (usize i = 0, n = tot_size; i < n; i++) {
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
        case Nst_IC_NO_OP:         PRINT("NO_OP        ", 13); break;
        case Nst_IC_POP_VAL:       PRINT("POP_VAL      ", 13); break;
        case Nst_IC_FOR_START:     PRINT("FOR_START    ", 13); break;
        case Nst_IC_RETURN_VAL:    PRINT("RETURN_VAL   ", 13); break;
        case Nst_IC_RETURN_VARS:   PRINT("RETURN_VARS  ", 13); break;
        case Nst_IC_SET_VAL_LOC:   PRINT("SET_VAL_LOC  ", 13); break;
        case Nst_IC_SET_CONT_LOC:  PRINT("SET_CONT_LOC ", 13); break;
        case Nst_IC_JUMP:          PRINT("JUMP         ", 13); break;
        case Nst_IC_JUMPIF_T:      PRINT("JUMPIF_T     ", 13); break;
        case Nst_IC_JUMPIF_F:      PRINT("JUMPIF_F     ", 13); break;
        case Nst_IC_JUMPIF_ZERO:   PRINT("JUMPIF_ZERO  ", 13); break;
        case Nst_IC_THROW_ERR:     PRINT("THROW_ERR    ", 13); break;
        case Nst_IC_PUSH_CATCH:    PRINT("PUSH_CATCH   ", 13); break;
        case Nst_IC_POP_CATCH:     PRINT("POP_CATCH    ", 13); break;
        case Nst_IC_SET_VAL:       PRINT("SET_VAL      ", 13); break;
        case Nst_IC_GET_VAL:       PRINT("GET_VAL      ", 13); break;
        case Nst_IC_PUSH_VAL:      PRINT("PUSH_VAL     ", 13); break;
        case Nst_IC_SET_CONT_VAL:  PRINT("SET_CONT_VAL ", 13); break;
        case Nst_IC_OP_CALL:       PRINT("OP_CALL      ", 13); break;
        case Nst_IC_OP_CAST:       PRINT("OP_CAST      ", 13); break;
        case Nst_IC_OP_RANGE:      PRINT("OP_RANGE     ", 13); break;
        case Nst_IC_STACK_OP:      PRINT("STACK_OP     ", 13); break;
        case Nst_IC_LOCAL_OP:      PRINT("LOCAL_OP     ", 13); break;
        case Nst_IC_OP_IMPORT:     PRINT("OP_IMPORT    ", 13); break;
        case Nst_IC_OP_EXTRACT:    PRINT("OP_EXTRACT   ", 13); break;
        case Nst_IC_DEC_INT:       PRINT("DEC_INT      ", 13); break;
        case Nst_IC_NEW_INT:       PRINT("NEW_INT      ", 13); break;
        case Nst_IC_DUP:           PRINT("DUP          ", 13); break;
        case Nst_IC_ROT_2:         PRINT("ROT_2        ", 13); break;
        case Nst_IC_ROT_3:         PRINT("ROT_3        ", 13); break;
        case Nst_IC_MAKE_ARR:      PRINT("MAKE_ARR     ", 13); break;
        case Nst_IC_MAKE_ARR_REP:  PRINT("MAKE_ARR_REP ", 13); break;
        case Nst_IC_MAKE_VEC:      PRINT("MAKE_VEC     ", 13); break;
        case Nst_IC_MAKE_VEC_REP:  PRINT("MAKE_VEC_REP ", 13); break;
        case Nst_IC_MAKE_MAP:      PRINT("MAKE_MAP     ", 13); break;
        case Nst_IC_MAKE_FUNC:     PRINT("MAKE_FUNC    ", 13); break;
        case Nst_IC_JUMPIF_IEND:   PRINT("FOR_IS_DONE  ", 13); break;
        case Nst_IC_FOR_NEXT:      PRINT("FOR_NEXT     ", 13); break;
        case Nst_IC_SAVE_ERROR:    PRINT("SAVE_ERROR   ", 13); break;
        case Nst_IC_UNPACK_SEQ:    PRINT("UNPACK_SEQ   ", 13); break;
        default:                   PRINT("__UNKNOWN__  ", 13); break;
        }

        PRINT(" |", 2);

        if (inst->code == Nst_IC_NO_OP) {
            PRINT("\n", 1);
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
            PRINT("\n", 1);
            continue;
        }

        if (code == Nst_IC_PUSH_VAL || code == Nst_IC_GET_VAL
            || code == Nst_IC_SET_VAL || code == Nst_IC_SET_VAL_LOC)
        {
            Nst_Obj *obj = Nst_ilist_get_inst_obj(ls, i);
            Nst_Obj *s = Nst_obj_to_repr_str(obj);
            if (Nst_error_occurred()) {
                Nst_error_clear();
                PRINT("\n", 1);
                continue;
            }
            Nst_printf(" [(%s) ", Nst_type_name(obj->type).value);
            Nst_fwrite(Nst_str_value(s), Nst_str_len(s), NULL, Nst_io.out);
            Nst_dec_ref(s);
            PRINT("]", 1);
        } else if (code == Nst_IC_MAKE_FUNC) {
            PRINT("\n\n", 2);
            for (i32 j = 0; j < indent + 1; j++)
                PRINT("    ", 4);
            PRINT("Function instructions:\n", 23);
            Nst_FuncPrototype *func = Nst_ilist_get_inst_func(ls, i);
            print_bytecode(&func->ilist, indent + 1);
        } else if (code == Nst_IC_STACK_OP || code == Nst_IC_LOCAL_OP) {
            PRINT(" [", 2);

            switch (inst->val) {
            case Nst_TT_ADD:    PRINT("+", 1);  break;
            case Nst_TT_SUB:    PRINT("-", 1);  break;
            case Nst_TT_MUL:    PRINT("*", 1);  break;
            case Nst_TT_DIV:    PRINT("/", 1);  break;
            case Nst_TT_POW:    PRINT("^", 1);  break;
            case Nst_TT_MOD:    PRINT("%", 1);  break;
            case Nst_TT_B_AND:  PRINT("&", 1);  break;
            case Nst_TT_B_OR:   PRINT("|", 1);  break;
            case Nst_TT_LEN:    PRINT("$", 1);  break;
            case Nst_TT_L_NOT:  PRINT("!", 1);  break;
            case Nst_TT_B_NOT:  PRINT("~", 1);  break;
            case Nst_TT_GT:     PRINT(">", 1);  break;
            case Nst_TT_LT:     PRINT("<", 1);  break;
            case Nst_TT_B_XOR:  PRINT("^^", 2); break;
            case Nst_TT_LSHIFT: PRINT("<<", 2); break;
            case Nst_TT_RSHIFT: PRINT(">>", 2); break;
            case Nst_TT_CONCAT: PRINT("><", 2); break;
            case Nst_TT_L_AND:  PRINT("&&", 2); break;
            case Nst_TT_L_OR:   PRINT("||", 2); break;
            case Nst_TT_L_XOR:  PRINT("&|", 2); break;
            case Nst_TT_EQ:     PRINT("==", 2); break;
            case Nst_TT_NEQ:    PRINT("!=", 2); break;
            case Nst_TT_GTE:    PRINT(">=", 2); break;
            case Nst_TT_LTE:    PRINT("<=", 2); break;
            case Nst_TT_NEG:    PRINT("-:", 2); break;
            case Nst_TT_STDOUT: PRINT(">>>", 3);break;
            case Nst_TT_STDIN:  PRINT("<<<", 3);break;
            case Nst_TT_TYPEOF: PRINT("?::", 3);break;
            case Nst_TT_CONTAINS:PRINT("<.>", 3);break;
            default: PRINT("__UNKNOWN_OP__", 14);
            }
            PRINT("]", 1);
        }
        PRINT("\n", 1);
    }
}

void Nst_ilist_print(Nst_InstList *ls)
{
    print_bytecode(ls, 0);
}
