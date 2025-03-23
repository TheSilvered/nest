#include <string.h>
#include "nest.h"

Nst_Inst *Nst_inst_new(Nst_InstID id, Nst_Pos start, Nst_Pos end)
{
    Nst_Inst *inst = Nst_malloc_c(1, Nst_Inst);
    if (inst == NULL)
        return NULL;

    inst->id = id;
    inst->int_val = 0;
    inst->val = NULL;
    inst->start = start;
    inst->end = end;

    Nst_assert_c(start.text != NULL);
    Nst_assert_c(end.text != NULL);

    return inst;
}

Nst_Inst *Nst_inst_new_val(Nst_InstID id, Nst_Obj *val, Nst_Pos start,
                            Nst_Pos end)
{
    Nst_Inst *inst = Nst_malloc_c(1, Nst_Inst);
    if (inst == NULL)
        return NULL;

    inst->id = id;
    inst->int_val = 0;
    inst->val = Nst_inc_ref(val);
    inst->start = start;
    inst->end = end;

    Nst_assert_c(start.text != NULL);
    Nst_assert_c(end.text != NULL);

    return inst;
}

Nst_Inst *Nst_inst_new_int(Nst_InstID id, i64 int_val, Nst_Pos start,
                           Nst_Pos end)
{
    Nst_Inst *inst = Nst_malloc_c(1, Nst_Inst);
    if (inst == NULL)
        return NULL;

    inst->id = id;
    inst->int_val = int_val;
    inst->val = NULL;
    inst->start = start;
    inst->end = end;

    Nst_assert_c(start.text != NULL);
    Nst_assert_c(end.text != NULL);

    return inst;
}

void Nst_inst_destroy(Nst_Inst *inst)
{
    if (inst->val != NULL)
        Nst_dec_ref(inst->val);
    Nst_free(inst);
}

Nst_InstList *Nst_inst_list_new(Nst_LList *instructions)
{
    Nst_InstList *inst_ls = Nst_malloc_c(1, Nst_InstList);
    if (inst_ls == NULL)
        return NULL;

    Nst_Inst *inst_array = Nst_malloc_c(instructions->len, Nst_Inst);
    if (inst_array == NULL) {
        Nst_free(inst_ls);
        return NULL;
    }
    inst_ls->instructions = inst_array;
    inst_ls->total_size = instructions->len;

    Nst_LList *functions = Nst_llist_new();
    if (functions == NULL) {
        Nst_free(inst_array);
        Nst_free(inst_ls);
        return false;
    }
    inst_ls->functions = functions;

    usize i = 0;
    for (Nst_LLIST_ITER(lnode, instructions)) {
        Nst_Inst *inst = (Nst_Inst *)(lnode->value);

        if (inst->id == Nst_IC_PUSH_VAL
            && inst->val != NULL
            && inst->val->type == Nst_t.Func)
        {
            Nst_inc_ref(inst->val);
            if (!Nst_llist_append(functions, inst->val, true))
                goto failure;
        }

        memcpy(inst_array + i, inst, sizeof(Nst_Inst));
        i++;
    }

    return inst_ls;

failure:
    Nst_llist_destroy(functions, (Nst_LListDestructor)Nst_dec_ref);
    Nst_free(inst_array);
    Nst_free(inst_ls);
    return NULL;
}

void Nst_inst_list_destroy(Nst_InstList *inst_list)
{
    Nst_Inst *instructions = inst_list->instructions;
    for (i64 i = 0, n = inst_list->total_size; i < n; i++) {
        if (instructions[i].val != NULL)
            Nst_dec_ref(instructions[i].val);
    }

    Nst_free(instructions);
    Nst_llist_destroy(inst_list->functions, (Nst_LListDestructor)Nst_dec_ref);
    Nst_free(inst_list);
}

#define PRINT(str, size) Nst_fwrite((u8 *)(str), (size), NULL, Nst_io.out)

static void print_bytecode(Nst_InstList *ls, i32 indent)
{
    usize tot_size = ls->total_size;
    int idx_width = 1;
    int max_col = 0;
    int max_row = 0;
    i64 max_int = 0;
    int col_width = 1;
    int row_width = 1;
    int int_width = 1;

    for (usize i = 0, n = ls->total_size; i < n; i++) {
        Nst_Inst inst = ls->instructions[i];
        if (inst.int_val > max_int)
            max_int = inst.int_val;
        if (inst.start.col > max_col)
            max_col = inst.start.col;
        if (inst.start.line > max_row)
            max_row = inst.start.line;
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
    while (max_int >= 10) {
        max_int /= 10;
        int_width++;
    }

    idx_width = idx_width < 3 ? 3 : idx_width;
    col_width = col_width < 3 ? 3 : col_width;
    row_width = row_width < 3 ? 3 : row_width;
    int_width = int_width < 3 ? 3 : int_width;

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
    PRINT("|  Instruction  |", 17);
    for (i32 i = 3; i < int_width; i++)
        PRINT(" ", 1);
    PRINT(" Int | Object\n", 14);

    for (usize i = 0, n = ls->total_size; i < n; i++) {
        Nst_Inst inst = ls->instructions[i];

        for (i32 j = 0; j < indent; j++)
            Nst_print("    ");
        Nst_printf(
            " %*zi | %*" PRIi32 ":%-*" PRIi32 " | ",
            idx_width, i,
            row_width,
            inst.start.line + 1,
            col_width,
            inst.start.col + 1);

        switch (inst.id) {
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
        case Nst_IC_HASH_CHECK:    PRINT("HASH_CHECK   ", 13); break;
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
        case Nst_IC_ROT:           PRINT("ROT          ", 13); break;
        case Nst_IC_MAKE_ARR:      PRINT("MAKE_ARR     ", 13); break;
        case Nst_IC_MAKE_ARR_REP:  PRINT("MAKE_ARR_REP ", 13); break;
        case Nst_IC_MAKE_VEC:      PRINT("MAKE_VEC     ", 13); break;
        case Nst_IC_MAKE_VEC_REP:  PRINT("MAKE_VEC_REP ", 13); break;
        case Nst_IC_MAKE_MAP:      PRINT("MAKE_MAP     ", 13); break;
        case Nst_IC_JUMPIF_IEND:   PRINT("FOR_IS_DONE  ", 13); break;
        case Nst_IC_FOR_NEXT:      PRINT("FOR_NEXT     ", 13); break;
        case Nst_IC_SAVE_ERROR:    PRINT("SAVE_ERROR   ", 13); break;
        case Nst_IC_UNPACK_SEQ:    PRINT("UNPACK_SEQ   ", 13); break;
        default:                   PRINT("__UNKNOWN__  ", 13); break;
        }

        if (inst.id == Nst_IC_NO_OP) {
            PRINT(" | ", 3);
            for (i32 j = 0; j < idx_width; j++)
                PRINT(" ", 1);
            PRINT(" |", 2);
            PRINT("\n", 1);
            continue;
        }

        if (Nst_INST_IS_JUMP(inst.id) || inst.id == Nst_IC_LOCAL_OP
            || inst.id == Nst_IC_STACK_OP || inst.id == Nst_IC_MAKE_ARR
            || inst.id == Nst_IC_MAKE_VEC || inst.id == Nst_IC_MAKE_MAP
            || inst.int_val != 0)
        {
            Nst_printf(" | %*" PRIi64 " |", idx_width, inst.int_val);
        } else {
            PRINT(" | ", 3);
            for (i32 j = 0; j < idx_width; j++)
                PRINT(" ", 1);
            PRINT(" |", 2);
        }

        if (inst.val != NULL) {
            Nst_printf(" (%s) ", Nst_type_name(inst.val->type).value);
            Nst_Obj *s = _Nst_repr_str_cast(inst.val);
            if (Nst_error_occurred())
                Nst_error_clear();
            else {
                Nst_fwrite(Nst_str_value(s), Nst_str_len(s), NULL, Nst_io.out);
                Nst_dec_ref(s);
            }

            if (inst.val->type == Nst_t.Func) {
                PRINT("\n\n", 2);
                for (i32 j = 0; j < indent + 1; j++)
                    PRINT("    ", 4);
                PRINT("<Func object> bytecode:\n", 24);
                print_bytecode(Nst_func_nest_body(inst.val), indent + 1);
            }
        } else if (inst.id == Nst_IC_STACK_OP || inst.id == Nst_IC_LOCAL_OP) {
            PRINT(" [", 2);

            switch (inst.int_val) {
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

void Nst_inst_list_print(Nst_InstList *ls)
{
    print_bytecode(ls, 0);
}
