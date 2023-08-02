#include <errno.h>
#include "mem.h"
#include "optimizer.h"
#include "obj_ops.h"
#include "tokens.h"
#include "error_internal.h"
#include "iter.h"
#include "hash.h"
#include "format.h"

#define HEAD_NODE Nst_NODE(node->nodes->head->value)
#define TAIL_NODE Nst_NODE(node->nodes->tail->value)

#define HEAD_TOK Nst_TOK(node->tokens->head->value)
#define TAIL_TOK Nst_TOK(node->tokens->tail->value)

static void ast_optimize_node(Nst_Node *node, Nst_Error *error);
static void ast_optimize_node_nodes(Nst_Node *node, Nst_Error *error);
static void ast_optimize_stack_op(Nst_Node *node, Nst_Error *error);
static void ast_optimize_comp_op(Nst_Node *node, Nst_Error *error);
static void ast_optimize_local_op(Nst_Node *node, Nst_Error *error);
static void ast_optimize_long_s(Nst_Node *node, Nst_Error *error);

Nst_Node *Nst_optimize_ast(Nst_Node *ast, Nst_Error *error)
{
    ast_optimize_node(ast, error);

    if (error->occurred) {
        Nst_node_destroy(ast);
        return NULL;
    }

    return ast;
}

static void ast_optimize_node(Nst_Node *node, Nst_Error *error)
{
    switch (node->type) {
    case Nst_NT_STACK_OP:
        ast_optimize_stack_op(node, error);
        break;
    case Nst_NT_LOCAL_OP:
        ast_optimize_local_op(node, error);
        break;
    case Nst_NT_LONG_S:
        ast_optimize_long_s(node, error);
    case Nst_NT_VALUE:
        break;
    default:
        ast_optimize_node_nodes(node, error);
    }
}

static void ast_optimize_node_nodes(Nst_Node *node, Nst_Error *error)
{
    for (Nst_LLIST_ITER(n, node->nodes)) {
        ast_optimize_node(Nst_NODE(n->value), error);
        if (error->occurred)
            return;
    }
}

static void ast_optimize_stack_op(Nst_Node *node, Nst_Error *error)
{
    if (Nst_IS_COMP_OP(HEAD_TOK->type)) {
        ast_optimize_comp_op(node, error);
        return;
    }

    ast_optimize_node(HEAD_NODE, error);
    ast_optimize_node(TAIL_NODE, error);

    if (error->occurred)
        return;

    if (HEAD_NODE->type != Nst_NT_VALUE || TAIL_NODE->type != Nst_NT_VALUE)
        return;

    i32 op_tok = HEAD_TOK->type;
    Nst_Obj *ob1 = Nst_TOK(HEAD_NODE->tokens->head->value)->value;
    Nst_Obj *ob2 = Nst_TOK(TAIL_NODE->tokens->head->value)->value;
    Nst_Obj *res = NULL;

    switch (op_tok) {
    case Nst_TT_ADD:    res = Nst_obj_add   (ob1, ob2); break;
    case Nst_TT_SUB:    res = Nst_obj_sub   (ob1, ob2); break;
    case Nst_TT_MUL:    res = Nst_obj_mul   (ob1, ob2); break;
    case Nst_TT_DIV:    res = Nst_obj_div   (ob1, ob2); break;
    case Nst_TT_POW:    res = Nst_obj_pow   (ob1, ob2); break;
    case Nst_TT_MOD:    res = Nst_obj_mod   (ob1, ob2); break;
    case Nst_TT_B_AND:  res = Nst_obj_bwand (ob1, ob2); break;
    case Nst_TT_B_OR:   res = Nst_obj_bwor  (ob1, ob2); break;
    case Nst_TT_B_XOR:  res = Nst_obj_bwxor (ob1, ob2); break;
    case Nst_TT_LSHIFT: res = Nst_obj_bwls  (ob1, ob2); break;
    case Nst_TT_RSHIFT: res = Nst_obj_bwrs  (ob1, ob2); break;
    case Nst_TT_CONCAT: res = Nst_obj_concat(ob1, ob2); break;
    case Nst_TT_L_AND:  res = Nst_obj_lgand (ob1, ob2); break;
    case Nst_TT_L_OR:   res = Nst_obj_lgor  (ob1, ob2); break;
    case Nst_TT_L_XOR:  res = Nst_obj_lgxor (ob1, ob2); break;
    }

    if (res == NULL) {
        _Nst_SET_ERROR_FROM_OP_ERR(error, node->start, node->end);
        return;
    }

    Nst_node_destroy(Nst_NODE(Nst_llist_pop(node->nodes)));
    Nst_node_destroy(Nst_NODE(Nst_llist_pop(node->nodes)));
    Nst_token_destroy(Nst_TOK(Nst_llist_peek_front(node->tokens)));

    node->type = Nst_NT_VALUE;

    Nst_Tok *new_tok = Nst_tok_new_value(
        node->start,
        node->end,
        Nst_TT_VALUE,
        res);

    if (new_tok == NULL) {
        _Nst_SET_ERROR_FROM_OP_ERR(error, node->start, node->end);
        return;
    }
    node->tokens->head->value = new_tok;
}

static void ast_optimize_comp_op(Nst_Node *node, Nst_Error *error)
{
    Nst_Obj *res = NULL;
    i32 op_tok = HEAD_TOK->type;
    Nst_Obj *ob1 = NULL;
    Nst_Obj *ob2 = NULL;

    ast_optimize_node_nodes(node, error);
    if (error->occurred)
        return;

    for (Nst_LLIST_ITER(n, node->nodes)) {
        if (Nst_NODE(n->value)->type != Nst_NT_VALUE)
            return;
    }

    for (Nst_LLIST_ITER(n, node->nodes)) {
        if (n->next == NULL)
            break;

        ob1 = Nst_TOK(Nst_NODE(n->value)->tokens->head->value)->value;
        ob2 = Nst_TOK(Nst_NODE(n->next->value)->tokens->head->value)->value;

        switch (op_tok) {
        case Nst_TT_GT:  res = Nst_obj_gt(ob1, ob2); break;
        case Nst_TT_LT:  res = Nst_obj_lt(ob1, ob2); break;
        case Nst_TT_EQ:  res = Nst_obj_eq(ob1, ob2); break;
        case Nst_TT_NEQ: res = Nst_obj_ne(ob1, ob2); break;
        case Nst_TT_GTE: res = Nst_obj_ge(ob1, ob2); break;
        case Nst_TT_LTE: res = Nst_obj_le(ob1, ob2); break;
        default: return;
        }

        if (res == Nst_c.Bool_false)
            break;
        else if (res == NULL) {
            _Nst_SET_ERROR_FROM_OP_ERR(error, node->start, node->end);
            return;
        }
    }

    Nst_llist_empty(node->nodes, (Nst_LListDestructor)Nst_node_destroy);
    Nst_token_destroy(Nst_TOK(Nst_llist_peek_front(node->tokens)));

    node->type = Nst_NT_VALUE;

    Nst_Tok *new_tok = Nst_tok_new_value(
        node->start,
        node->end,
        Nst_TT_VALUE,
        res);

    if (new_tok == NULL) {
        _Nst_SET_ERROR_FROM_OP_ERR(error, node->start, node->end);
        return;
    }
    node->tokens->head->value = new_tok;
}

static void ast_optimize_local_op(Nst_Node *node, Nst_Error *error)
{
    ast_optimize_node(HEAD_NODE, error);

    if (error->occurred)
        return;

    if (HEAD_NODE->type != Nst_NT_VALUE)
        return;

    i32 op_tok = HEAD_TOK->type;
    Nst_Obj *ob = Nst_TOK(HEAD_NODE->tokens->head->value)->value;
    Nst_Obj *res = NULL;

    switch (op_tok) {
    case Nst_TT_LEN:    res = Nst_obj_len   (ob); break;
    case Nst_TT_L_NOT:  res = Nst_obj_lgnot (ob); break;
    case Nst_TT_B_NOT:  res = Nst_obj_bwnot (ob); break;
    case Nst_TT_TYPEOF: res = Nst_obj_typeof(ob); break;
    case Nst_TT_NEG:    res = Nst_obj_neg   (ob); break;
    default: return;
    }

    if (res == NULL) {
        _Nst_SET_ERROR_FROM_OP_ERR(error, node->start, node->end);
        return;
    }

    Nst_node_destroy(Nst_NODE(Nst_llist_pop(node->nodes)));
    Nst_token_destroy(Nst_TOK(Nst_llist_peek_front(node->tokens)));

    node->type = Nst_NT_VALUE;

    Nst_Tok *new_tok = Nst_tok_new_value(
        node->start,
        node->end,
        Nst_TT_VALUE,
        res);

    if (new_tok == NULL) {
        _Nst_SET_ERROR_FROM_OP_ERR(error, node->start, node->end);
        return;
    }
    node->tokens->head->value = new_tok;
}

static void ast_optimize_long_s(Nst_Node *node, Nst_Error *error)
{
    Nst_LList *nodes = node->nodes;
    Nst_LLNode *prev_valid_node = NULL;
    Nst_Node *curr_node;

    for (Nst_LLIST_ITER(n, node->nodes)) {
        ast_optimize_node(Nst_NODE(n->value), error);
        if (error->occurred)
            return;

        curr_node = Nst_NODE(n->value);

        if (curr_node->type != Nst_NT_VALUE
            && curr_node->type != Nst_NT_ACCESS)
        {
            prev_valid_node = n;
            continue;
        }

        if (prev_valid_node == NULL) {
            nodes->head = n->next;
            if (n->next == NULL)
                nodes->tail = NULL;
        } else {
            prev_valid_node->next = n->next;
            if (n->next == NULL)
                nodes->tail = prev_valid_node;
        }

        Nst_node_destroy(Nst_NODE(n->value));
        Nst_free(n);

        if (prev_valid_node == NULL)
            n = nodes->head;
        else
            n = prev_valid_node;

        if (n == NULL)
            break;
    }
}

static bool can_optimize_consts(Nst_InstList *bc);
static bool is_accessed(Nst_InstList *bc, Nst_StrObj *name);
static bool has_assignments(Nst_InstList *bc, Nst_StrObj *name);
static bool has_jumps_to(Nst_InstList *bc, Nst_Int idx, Nst_Int avoid_start,
                         Nst_Int avoid_end);
static void replace_access(Nst_InstList *bc, Nst_StrObj *name, Nst_Obj *val);
static void optimize_const(Nst_InstList *bc, const i8 *name, Nst_Obj *val);
static void remove_push_pop(Nst_InstList *bc);
static void remove_assign_pop(Nst_InstList *bc);
static void remove_assign_loc_get_val(Nst_InstList *bc);
static void remove_push_check(Nst_InstList *bc, Nst_Error *error);
static void remove_push_jumpif(Nst_InstList *bc);
static void remove_inst(Nst_InstList *bc, Nst_Int idx);
static void optimize_funcs(Nst_InstList *bc, Nst_Error *error);
static void remove_dead_code(Nst_InstList *bc);
static void optimize_chained_jumps(Nst_InstList *bc);

Nst_InstList *Nst_optimize_bytecode(Nst_InstList *bc, bool optimize_builtins,
                                    Nst_Error *error)
{
    if (optimize_builtins && can_optimize_consts(bc)) {
        optimize_const(bc, "Type",   OBJ(Nst_t.Type));
        optimize_const(bc, "Int",    OBJ(Nst_t.Int));
        optimize_const(bc, "Real",   OBJ(Nst_t.Real));
        optimize_const(bc, "Bool",   OBJ(Nst_t.Bool));
        optimize_const(bc, "Null",   OBJ(Nst_t.Null));
        optimize_const(bc, "Str",    OBJ(Nst_t.Str));
        optimize_const(bc, "Array",  OBJ(Nst_t.Array));
        optimize_const(bc, "Vector", OBJ(Nst_t.Vector));
        optimize_const(bc, "Map",    OBJ(Nst_t.Map));
        optimize_const(bc, "Func",   OBJ(Nst_t.Func));
        optimize_const(bc, "Iter",   OBJ(Nst_t.Iter));
        optimize_const(bc, "Byte",   OBJ(Nst_t.Byte));
        optimize_const(bc, "IOFile", OBJ(Nst_t.IOFile));
        optimize_const(bc, "true",   OBJ(Nst_c.Bool_true));
        optimize_const(bc, "false",  OBJ(Nst_c.Bool_false));
        optimize_const(bc, "null",   OBJ(Nst_c.Null_null));
    }

    Nst_Int initial_size;
    do {
        initial_size = bc->total_size;
        remove_push_check(bc, error);
        if (error->occurred) {
            Nst_inst_list_destroy(bc);
            return NULL;
        }

        remove_push_pop(bc);
        remove_assign_pop(bc);
        remove_assign_loc_get_val(bc);
        remove_push_jumpif(bc);
        optimize_chained_jumps(bc);
        remove_dead_code(bc);

        optimize_funcs(bc, error);
        if (error->occurred) {
            Nst_inst_list_destroy(bc);
            return NULL;
        }

        Nst_Int size = bc->total_size;
        Nst_Inst *inst_list = bc->instructions;
        for (Nst_Int i = 0; i < size; i++) {
            if (inst_list[i].id == Nst_IC_NO_OP) {
                remove_inst(bc, i);
                i--;
                size--;
            }
        }
    } while (initial_size != (Nst_Int)bc->total_size);

    return bc;
}

static bool can_optimize_consts(Nst_InstList *bc)
{
    // if _vars_, _globals_, *._vars_, *._globals_ are set to other
    // variables explicitly (_vars_ = x) or implicitly (_vars_ @x)
    // predefined constants cannot be optimized
    if (has_assignments(bc, Nst_s.o__vars_)
        || has_assignments(bc, Nst_s.o__globals_))
    {
        return false;
    }

    if (is_accessed(bc, Nst_s.o__vars_)
        || is_accessed(bc, Nst_s.o__globals_))
    {
        return false;
    }

    return true;
}

static bool is_accessed(Nst_InstList *bc, Nst_StrObj *name)
{
    // if the name is followed by any number of LOCAL_OP, TYPE_CHECK,
    // HASH_CHECK and then POP_VAL, JUMPIF_T or JUMPIF_F
    // or is followed by PUSH_VAL and OP_EXTRACT or SET_CONT_VAL
    // it is not counted as an access

    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (Nst_Int i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_PUSH_VAL
            && inst_list[i].val->type == Nst_t.Str
            && Nst_string_compare(STR(inst_list[i].val), name) == 0 )
        {
            if (inst_list[++i].id == Nst_IC_OP_EXTRACT)
                return true;
            continue;
        }

        if (inst_list[i].id != Nst_IC_GET_VAL
            || Nst_string_compare(STR(inst_list[i].val), name) != 0)
        {
            continue;
        }

        if (inst_list[i + 1].id == Nst_IC_LOCAL_OP
            || inst_list[i + 1].id == Nst_IC_HASH_CHECK
            || inst_list[i + 1].id == Nst_IC_TYPE_CHECK)
        {
            i += 2;

            while (i < size
                   && (inst_list[i].id == Nst_IC_LOCAL_OP
                       || inst_list[i].id == Nst_IC_HASH_CHECK
                       || inst_list[i].id == Nst_IC_TYPE_CHECK))
            {
                i++;
            }

            if (i == size
                || (inst_list[i].id != Nst_IC_POP_VAL
                    && inst_list[i].id != Nst_IC_JUMPIF_T
                    && inst_list[i].id != Nst_IC_JUMPIF_F))
            {
                return true;
            }
        } else if (inst_list[i + 1].id == Nst_IC_PUSH_VAL) {
            i += 2;

            if (i == size
                || (inst_list[i].id != Nst_IC_OP_EXTRACT
                    && inst_list[i].id != Nst_IC_SET_CONT_VAL))
            {
                return true;
            }
            i -= 2;
        } else
            return true;
    }

    for (Nst_LLIST_ITER(n, bc->functions)) {
        if (is_accessed(FUNC(n->value)->body.bytecode, name))
            return true;
    }

    return false;
}

static bool has_assignments(Nst_InstList *bc, Nst_StrObj *name)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (Nst_Int i = 0; i < size; i++) {
        if (i > 0 && inst_list[i].id == Nst_IC_SET_CONT_VAL) {
            Nst_Inst prev_inst = inst_list[i - 1];
            if (prev_inst.id == Nst_IC_PUSH_VAL
                && prev_inst.val->type == Nst_t.Str
                && Nst_string_compare(name, STR(prev_inst.val)) == 0)
            {
                return true;
            }
        }

        if (inst_list[i].id != Nst_IC_SET_VAL
            && inst_list[i].id != Nst_IC_SET_VAL_LOC)
        {
            continue;
        }

        if (Nst_string_compare(name, STR(inst_list[i].val)) == 0)
            return true;
    }

    for (Nst_LLIST_ITER(n, bc->functions)) {
        if (has_assignments(FUNC(n->value)->body.bytecode, name))
            return true;
    }

    return false;
}

static void replace_access(Nst_InstList *bc, Nst_StrObj *name, Nst_Obj *val)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (Nst_Int i = 0; i < size; i++) {
        if (inst_list[i].id != Nst_IC_GET_VAL)
            continue;

        if (Nst_string_compare(name, STR(inst_list[i].val)) == 0) {
            inst_list[i].id = Nst_IC_PUSH_VAL;
            Nst_dec_ref(inst_list[i].val);
            inst_list[i].val = Nst_inc_ref(val);
        }
    }

    for (Nst_LLIST_ITER(n, bc->functions))
        replace_access(FUNC(n->value)->body.bytecode, name, val);
}

static void optimize_const(Nst_InstList *bc, const i8 *name, Nst_Obj *val)
{
    Nst_StrObj *str_obj = STR(Nst_string_new_c_raw(name, false));
    if (str_obj == NULL) {
        Nst_error_clear();
        return;
    }

    if (has_assignments(bc, str_obj))
        goto end;

    replace_access(bc, str_obj, val);

end:
    Nst_dec_ref(str_obj);
}

static bool has_jumps_to(Nst_InstList *bc, Nst_Int idx, Nst_Int avoid_start,
                         Nst_Int avoid_end)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    Nst_Inst inst;

    for (Nst_Int i = 0; i < size; i++) {
        if (i >= avoid_start && i <= avoid_end)
            continue;

        inst = inst_list[i];
        if (Nst_INST_IS_JUMP(inst.id) && inst.int_val == idx)
            return true;
    }

    return false;
}

static void remove_push_pop(Nst_InstList *bc)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool expect_pop = false;

    for (Nst_Int i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_PUSH_VAL) {
            expect_pop = true;
            continue;
        } else if (!expect_pop
                   || inst_list[i].id != Nst_IC_POP_VAL
                   || has_jumps_to(bc, i, -1, -1) )
        {
            expect_pop = false;
            continue;
        }

        inst_list[i].id = Nst_IC_NO_OP;
        inst_list[i - 1].id = Nst_IC_NO_OP;
        Nst_dec_ref(inst_list[i - 1].val);
        inst_list[i - 1].val = NULL;

        expect_pop = false;
    }
}

static void remove_assign_pop(Nst_InstList *bc)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool expect_pop = false;

    for (Nst_Int i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_SET_VAL
            || inst_list[i].id == Nst_IC_SET_CONT_VAL)
        {
            expect_pop = true;
            continue;
        } else if (!expect_pop
                   || inst_list[i].id != Nst_IC_POP_VAL
                   || has_jumps_to(bc, i, -1, -1))
        {
            expect_pop = false;
            continue;
        }

        inst_list[i].id = Nst_IC_NO_OP;

        if (inst_list[i - 1].id == Nst_IC_SET_VAL)
            inst_list[i - 1].id = Nst_IC_SET_VAL_LOC;
        else
            inst_list[i - 1].id = Nst_IC_SET_CONT_LOC;

        expect_pop = false;
    }
}

static void remove_assign_loc_get_val(Nst_InstList* bc)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool expect_get_val = false;
    Nst_StrObj *expected_name = NULL;

    for (Nst_Int i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_SET_VAL_LOC) {
            expect_get_val = true;
            expected_name = STR(inst_list[i].val);
            continue;
        } else if (!expect_get_val
                   || inst_list[i].id != Nst_IC_GET_VAL
                   || has_jumps_to(bc, i, -1, -1)
                   || Nst_string_compare(
                        expected_name,
                        STR(inst_list[i].val)) != 0)
        {
            expect_get_val = false;
            expected_name = NULL;
            continue;
        }

        if (Nst_string_compare(expected_name, STR(inst_list[i].val)) != 0) {
            expect_get_val = false;
            expected_name = NULL;
            continue;
        }

        inst_list[i].id = Nst_IC_NO_OP;
        Nst_dec_ref(inst_list[i].val);
        inst_list[i].val = NULL;
        inst_list[i - 1].id = Nst_IC_SET_VAL;

        expect_get_val = false;
        expected_name = NULL;
    }
}

static void remove_push_check(Nst_InstList *bc, Nst_Error *error)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool was_push = false;

    for (Nst_Int i = 0; i < size; i++) {
        if (inst_list[i].id == Nst_IC_PUSH_VAL) {
            was_push = true;
            continue;
        } else if (!was_push
                   || (inst_list[i].id != Nst_IC_TYPE_CHECK
                       && inst_list[i].id != Nst_IC_HASH_CHECK)
                   || has_jumps_to(bc, i, -1, -1))
        {
            was_push = false;
            continue;
        }

        if (inst_list[i].id == Nst_IC_TYPE_CHECK) {
            if (inst_list[i].val != OBJ(inst_list[i - 1].val->type)) {
                _Nst_SET_TYPE_ERROR(
                    error,
                    inst_list[i].start,
                    inst_list[i].end,
                    Nst_sprintf(
                        _Nst_EM_EXPECTED_TYPES,
                        STR(inst_list[i].val)->value,
                        TYPE_NAME(inst_list[i - 1].val)));

                return;
            }
        } else {
            Nst_obj_hash(inst_list[i - 1].val);
            if (inst_list[i - 1].val->hash == -1) {
                _Nst_SET_TYPE_ERROR(
                    error,
                    inst_list[i].start,
                    inst_list[i].end,
                    Nst_sprintf(
                        _Nst_EM_UNHASHABLE_TYPE,
                        TYPE_NAME(inst_list[i - 1].val)));

                return;
            }
        }

        inst_list[i].id = Nst_IC_NO_OP;
        if (inst_list[i].val != NULL)
            Nst_dec_ref(inst_list[i].val);
        inst_list[i].val = NULL;
        was_push = false;
    }
    return;
}

static void remove_push_jumpif(Nst_InstList *bc)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;
    bool expect_jumpif = false;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( inst_list[i].id == Nst_IC_PUSH_VAL )
        {
            expect_jumpif = true;
            continue;
        }
        else if ( !expect_jumpif ||
                  !Nst_INST_IS_JUMP(inst_list[i].id) ||
                  inst_list[i].id == Nst_IC_JUMP ||
                  inst_list[i].id == Nst_IC_JUMPIF_ZERO ||
                  has_jumps_to(bc, i, -1, -1) )
        {
            expect_jumpif = false;
            continue;
        }

        Nst_Obj *cond = Nst_obj_cast(inst_list[i - 1].val, Nst_t.Bool);
        if ((cond == Nst_c.Bool_false && inst_list[i].id == Nst_IC_JUMPIF_F)
            || (cond == Nst_c.Bool_true && inst_list[i].id == Nst_IC_JUMPIF_T))
        {
            inst_list[i].id = Nst_IC_JUMP;
        } else
            inst_list[i].id = Nst_IC_NO_OP;

        Nst_dec_ref(inst_list[i - 1].val);
        inst_list[i - 1].val = NULL;
        inst_list[i - 1].id = Nst_IC_NO_OP;

        Nst_dec_ref(cond);
        expect_jumpif = false;
    }
}

static void remove_inst(Nst_InstList *bc, Nst_Int idx)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    if (idx < 0 || idx >= size)
        return;

    for (Nst_Int i = idx; i < size - 1; i++)
        inst_list[i] = inst_list[i + 1];

    bc->total_size--;
    size--;

    for (Nst_Int i = 0; i < size; i++) {
        if (Nst_INST_IS_JUMP(inst_list[i].id) && inst_list[i].int_val > idx)
            inst_list[i].int_val--;
    }
}

static void optimize_funcs(Nst_InstList *bc, Nst_Error *error)
{
    for (Nst_LLIST_ITER(n, bc->functions)) {
        Nst_optimize_bytecode(FUNC(n->value)->body.bytecode, false, error);

        if (error->occurred)
            break;
    }
}

static void remove_dead_code(Nst_InstList *bc)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (Nst_Int i = 0; i < size; i++) {
        if ((inst_list[i].id != Nst_IC_JUMP
             || i + 1 > inst_list[i].int_val)
            && inst_list[i].id != Nst_IC_RETURN_VAL
            && inst_list[i].id != Nst_IC_THROW_ERR)
        {
            continue;
        }

        bool is_jump_useless = inst_list[i].id == Nst_IC_JUMP;
        bool stop_at_save_error = inst_list[i].id == Nst_IC_THROW_ERR;
        Nst_Int end = is_jump_useless ? inst_list[i].int_val : size;

        for (Nst_Int j = i + 1; j < end; j++) {
            if (has_jumps_to(bc, j, i + 1, inst_list[i].int_val - 1)) {
                is_jump_useless = false;
                break;
            }

            if (stop_at_save_error && inst_list[j].id == Nst_IC_SAVE_ERROR)
                break;

            if (inst_list[j].val != NULL)
                Nst_dec_ref(inst_list[j].val);

            inst_list[j].id = Nst_IC_NO_OP;
            inst_list[j].val = NULL;
        }

        if (is_jump_useless)
            inst_list[i].id = Nst_IC_NO_OP;
    }
}

static void optimize_chained_jumps(Nst_InstList* bc)
{
    Nst_Int size = bc->total_size;
    Nst_Inst *inst_list = bc->instructions;

    for (Nst_Int i = 0; i < size; i++) {
        if (!Nst_INST_IS_JUMP(inst_list[i].id))
            continue;

        Nst_Int end_jump = inst_list[i].int_val;

        while (inst_list[end_jump].id == Nst_IC_JUMP)
            end_jump = inst_list[end_jump].int_val;

        inst_list[i].int_val = end_jump;
    }
}
