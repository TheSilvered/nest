#include <errno.h>
#include "optimizer.h"
#include "obj_ops.h"
#include "tokens.h"
#include "error_internal.h"
#include "iter.h"
#include "hash.h"

#define HEAD_NODE NODE(node->nodes->head->value)
#define TAIL_NODE NODE(node->nodes->tail->value)

#define HEAD_TOK TOK(node->tokens->head->value)
#define TAIL_TOK TOK(node->tokens->tail->value)

#define SET_ERROR(err_macro, start, end, message) \
    do { \
        Nst_Error *new_error = malloc(sizeof(Nst_Error)); \
        if ( new_error == NULL ) \
        { \
            errno = ENOMEM; \
            return; \
        } \
        err_macro(new_error, start, end, message); \
        *error = new_error; \
    } while ( 0 )

static void ast_optimize_node(Nst_Node *node, Nst_Error **error);
static void ast_optimize_node_nodes(Nst_Node *node, Nst_Error **error);
static void ast_optimize_stack_op(Nst_Node *node, Nst_Error **error);
static void ast_optimize_local_op(Nst_Node *node, Nst_Error **error);
static void ast_optimize_long_s(Nst_Node *node, Nst_Error **error);

Nst_Node *nst_optimize_ast(Nst_Node *ast)
{
    Nst_Error *error = NULL;
    ast_optimize_node(ast, &error);

    if ( error != NULL )
    {
        nst_print_error(*error);
        nst_destroy_node(ast);
        return NULL;
    }

    return ast;
}

static void ast_optimize_node(Nst_Node *node, Nst_Error **error)
{
    switch ( node->type )
    {
    case NST_NT_STACK_OP:
        ast_optimize_stack_op(node, error);
        break;
    case NST_NT_LOCAL_OP:
        ast_optimize_local_op(node, error);
        break;
    case NST_NT_LONG_S:
        ast_optimize_long_s(node, error);
    case NST_NT_VALUE:
        break;
    default:
        ast_optimize_node_nodes(node, error);
    }
}

static void ast_optimize_node_nodes(Nst_Node *node, Nst_Error **error)
{
    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        ast_optimize_node(n->value, error);
        if ( *error != NULL )
            return;
    }
}

static void ast_optimize_stack_op(Nst_Node *node, Nst_Error **error)
{
    ast_optimize_node(HEAD_NODE, error);
    ast_optimize_node(TAIL_NODE, error);

    if ( *error != NULL )
        return;

    if ( HEAD_NODE->type != NST_NT_VALUE || TAIL_NODE->type != NST_NT_VALUE )
        return;

    int op_tok = HEAD_TOK->type;
    Nst_OpErr err = { "", "" };
    Nst_Obj *ob1 = TOK(HEAD_NODE->tokens->head->value)->value;
    Nst_Obj *ob2 = TOK(TAIL_NODE->tokens->head->value)->value;
    Nst_Obj *res = NULL;

    switch ( op_tok ) {
    case NST_TT_ADD:    res = nst_obj_add(ob1, ob2, &err);   break;
    case NST_TT_SUB:    res = nst_obj_sub(ob1, ob2, &err);   break;
    case NST_TT_MUL:    res = nst_obj_mul(ob1, ob2, &err);   break;
    case NST_TT_DIV:    res = nst_obj_div(ob1, ob2, &err);   break;
    case NST_TT_POW:    res = nst_obj_pow(ob1, ob2, &err);   break;
    case NST_TT_MOD:    res = nst_obj_mod(ob1, ob2, &err);   break;
    case NST_TT_B_AND:  res = nst_obj_bwand(ob1, ob2, &err); break;
    case NST_TT_B_OR:   res = nst_obj_bwor(ob1, ob2, &err);  break;
    case NST_TT_B_XOR:  res = nst_obj_bwxor(ob1, ob2, &err); break;
    case NST_TT_LSHIFT: res = nst_obj_bwls(ob1, ob2, &err);  break;
    case NST_TT_RSHIFT: res = nst_obj_bwrs(ob1, ob2, &err);  break;
    case NST_TT_CONCAT: res = nst_obj_concat(ob1, ob2, &err);break;
    case NST_TT_L_AND:  res = nst_obj_lgand(ob1, ob2, &err); break;
    case NST_TT_L_OR:   res = nst_obj_lgor(ob1, ob2, &err);  break;
    case NST_TT_L_XOR:  res = nst_obj_lgxor(ob1, ob2, &err); break;
    case NST_TT_GT:     res = nst_obj_gt(ob1, ob2, &err);    break;
    case NST_TT_LT:     res = nst_obj_lt(ob1, ob2, &err);    break;
    case NST_TT_EQ:     res = nst_obj_eq(ob1, ob2, &err);    break;
    case NST_TT_NEQ:    res = nst_obj_ne(ob1, ob2, &err);    break;
    case NST_TT_GTE:    res = nst_obj_ge(ob1, ob2, &err);    break;
    case NST_TT_LTE:    res = nst_obj_le(ob1, ob2, &err);    break;
    default: return;
    }

    if ( res == NULL )
    {
        if ( errno == ENOMEM )
            return;
        SET_ERROR(_NST_SET_GENERAL_ERROR, node->start, node->end, err.message);
        (*error)->name = err.name;
        return;
    }

    nst_destroy_node(LList_pop(node->nodes));
    nst_destroy_node(LList_pop(node->nodes));
    nst_destroy_token(LList_pop(node->tokens));

    node->type = NST_NT_VALUE;

    Nst_LexerToken *new_tok = nst_new_token_value(
        node->start,
        node->end,
        NST_TT_VALUE,
        res
    );

    LList_append(node->tokens, new_tok, true);
}

static void ast_optimize_local_op(Nst_Node *node, Nst_Error **error)
{
    ast_optimize_node(HEAD_NODE, error);

    if ( *error != NULL )
        return;

    if ( HEAD_NODE->type != NST_NT_VALUE )
        return;

    int op_tok = HEAD_TOK->type;
    Nst_OpErr err = { "", "" };
    Nst_Obj *ob = TOK(HEAD_NODE->tokens->head->value)->value;
    Nst_Obj *res = NULL;

    switch ( op_tok )
    {
    case NST_TT_LEN:    res = nst_obj_len(ob, &err);    break;
    case NST_TT_L_NOT:  res = nst_obj_lgnot(ob, &err);  break;
    case NST_TT_B_NOT:  res = nst_obj_bwnot(ob, &err);  break;
    case NST_TT_TYPEOF: res = nst_obj_typeof(ob, &err); break;
    case NST_TT_NEG:    res = nst_obj_neg(ob, &err);    break;
    default: return;
    }

    if ( res == NULL )
    {
        if ( errno == ENOMEM )
            return;
        SET_ERROR(_NST_SET_GENERAL_ERROR, node->start, node->end, err.message);
        (*error)->name = err.name;
        return;
    }

    nst_destroy_node(LList_pop(node->nodes));
    nst_destroy_token(LList_pop(node->tokens));

    node->type = NST_NT_VALUE;

    LList_append(
        node->tokens,
        nst_new_token_value(
            node->start,
            node->end,
            NST_TT_VALUE,
            res
        ),
        true
    );
}

static void ast_optimize_long_s(Nst_Node *node, Nst_Error **error)
{
    LList *nodes = node->nodes;
    LLNode *prev_valid_node = NULL;
    Nst_Node *curr_node;

    for ( LLNode *n = node->nodes->head; n != NULL; n = n->next )
    {
        ast_optimize_node(n->value, error);
        if ( *error != NULL )
            return;

        curr_node = n->value;

        if ( curr_node->type != NST_NT_VALUE && curr_node->type != NST_NT_ACCESS )
        {
            prev_valid_node = n;
            continue;
        }

        if ( prev_valid_node == NULL )
        {
            nodes->head = n->next;
            if ( n->next == NULL )
                nodes->tail = NULL;
        }
        else
        {
            prev_valid_node->next = n->next;
            if ( n->next == NULL )
                nodes->tail = prev_valid_node;
        }

        nst_destroy_node(n->value);
        free(n);

        if ( prev_valid_node == NULL )
            n = nodes->head;
        else
            n = prev_valid_node;

        if ( n == NULL )
            break;
    }
}

static Nst_Int count_assignments(Nst_InstructionList *bc, Nst_StrObj *name);
static Nst_Int count_jumps_to(Nst_InstructionList *bc,
                              Nst_Int idx,
                              Nst_Int avoid_start,
                              Nst_Int avoid_end);
static void replace_access(Nst_InstructionList *bc, Nst_StrObj *name, Nst_Obj *val);
static void optimize_builtin(Nst_InstructionList *bc, char *name, Nst_Obj *val);
static void optimize_func_builtin(Nst_InstructionList *bc, Nst_StrObj *name, Nst_Obj *val);
static void remove_push_pop(Nst_InstructionList *bc);
static void remove_assign_pop(Nst_InstructionList *bc);
static bool remove_push_check(Nst_InstructionList *bc);
static void remove_push_jumpif(Nst_InstructionList *bc);
static void remove_inst(Nst_InstructionList *bc, Nst_Int idx);
static void optimize_funcs(Nst_InstructionList *bc);
static void remove_dead_code(Nst_InstructionList *bc);

Nst_InstructionList *nst_optimize_bytecode(Nst_InstructionList *bc, bool optimize_builtins)
{
    if ( optimize_builtins )
    {
        optimize_builtin(bc, "Type",   nst_t_type);
        optimize_builtin(bc, "Int",    nst_t_int );
        optimize_builtin(bc, "Real",   nst_t_real);
        optimize_builtin(bc, "Bool",   nst_t_bool);
        optimize_builtin(bc, "Null",   nst_t_null);
        optimize_builtin(bc, "Str",    nst_t_str );
        optimize_builtin(bc, "Array",  nst_t_arr );
        optimize_builtin(bc, "Vector", nst_t_vect);
        optimize_builtin(bc, "Map",    nst_t_map );
        optimize_builtin(bc, "Func",   nst_t_func);
        optimize_builtin(bc, "Iter",   nst_t_iter);
        optimize_builtin(bc, "Byte",   nst_t_byte);
        optimize_builtin(bc, "IOfile", nst_t_file);
        optimize_builtin(bc, "true",   nst_true );
        optimize_builtin(bc, "false",  nst_false);
        optimize_builtin(bc, "null",   nst_null );
    }

    Nst_Int initial_size;
    do
    {
        initial_size = bc->total_size;
        if ( remove_push_check(bc) )
        {
            nst_destroy_inst_list(bc);
            return NULL;
        }

        remove_push_pop(bc);
        remove_assign_pop(bc);
        remove_push_jumpif(bc);
        remove_dead_code(bc);
        optimize_funcs(bc);

        register Nst_Int size = bc->total_size;
        register Nst_RuntimeInstruction *inst_list = bc->instructions;
        for ( Nst_Int i = 0; i < size; i++ )
        {
            if ( inst_list[i].id == NST_IC_NO_OP )
            {
                remove_inst(bc, i);
                --i;
                --size;
            }
        }
    }
    while ( initial_size != (Nst_Int)bc->total_size );

    return bc;
}

static Nst_Int count_assignments(Nst_InstructionList *bc, Nst_StrObj *name)
{
    register Nst_Int tot = 0;
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( inst_list[i].id != NST_IC_SET_VAL &&
             inst_list[i].id != NST_IC_SET_VAL_LOC )
            continue;

        if ( strcmp(name->value, AS_STR(inst_list[i].val)->value) == 0 )
            ++tot;
    }

    return tot;
}

static void replace_access(Nst_InstructionList *bc, Nst_StrObj *name, Nst_Obj *val)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( inst_list[i].id != NST_IC_GET_VAL )
            continue;

        if ( strcmp(name->value, AS_STR(inst_list[i].val)->value) == 0 )
        {
            inst_list[i].id = NST_IC_PUSH_VAL;
            nst_dec_ref(inst_list[i].val);
            inst_list[i].val = nst_inc_ref(val);
        }
    }
}

static void optimize_builtin(Nst_InstructionList *bc, char *name, Nst_Obj *val)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;

    Nst_StrObj *str_obj = AS_STR(nst_new_string_raw(name, false));
    if ( count_assignments(bc, str_obj) != 0 )
        return;

    replace_access(bc, str_obj, val);

    for ( Nst_Int i = 0; i < size - 1; i++ )
    {
        if ( inst_list[i].id == NST_IC_PUSH_VAL &&
             inst_list[i].val != NULL &&
             inst_list[i].val->type == nst_t_func )
            optimize_func_builtin(AS_FUNC(inst_list[i].val)->body, str_obj, val);
    }

    nst_dec_ref(str_obj);
}

static void optimize_func_builtin(Nst_InstructionList *bc, Nst_StrObj *name, Nst_Obj *val)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;

    if ( count_assignments(bc, name) == 0 )
        replace_access(bc, name, val);

    for ( Nst_Int i = 0; i < size - 1; i++ )
    {
        if ( inst_list[i].id == NST_IC_PUSH_VAL &&
             inst_list[i].val != NULL &&
             inst_list[i].val->type == nst_t_func )
            optimize_func_builtin(AS_FUNC(inst_list[i].val)->body, name, val);
    }
}

static Nst_Int count_jumps_to(Nst_InstructionList *bc,
                              Nst_Int idx,
                              Nst_Int avoid_start,
                              Nst_Int avoid_end)
{
    register Nst_Int tot = 0;
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;
    register Nst_RuntimeInstruction inst;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( i >= avoid_start && i <= avoid_end )
            continue;

        inst = inst_list[i];
        if ( IS_JUMP(inst.id) && inst.int_val == idx )
            ++tot;
    }

    return tot;
}

static void remove_push_pop(Nst_InstructionList *bc)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;
    register bool expect_pop = false;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( inst_list[i].id == NST_IC_PUSH_VAL )
        {
            expect_pop = true;
            continue;
        }
        else if ( !expect_pop ||
                  inst_list[i].id != NST_IC_POP_VAL ||
                  count_jumps_to(bc, i, -1, -1) != 0 )
        {
            expect_pop = false;
            continue;
        }

        inst_list[i].id = NST_IC_NO_OP;
        inst_list[i - 1].id = NST_IC_NO_OP;

        expect_pop = false;
    }
}

static void remove_assign_pop(Nst_InstructionList *bc)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;
    register bool expect_pop = false;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( inst_list[i].id == NST_IC_SET_VAL )
        {
            expect_pop = true;
            continue;
        }
        else if ( !expect_pop ||
                  inst_list[i].id != NST_IC_POP_VAL ||
                  count_jumps_to(bc, i, -1, -1) != 0 )
        {
            expect_pop = false;
            continue;
        }

        inst_list[i].id = NST_IC_NO_OP;
        inst_list[i - 1].id = NST_IC_SET_VAL_LOC;

        expect_pop = false;
    }
}

static bool remove_push_check(Nst_InstructionList *bc)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;
    register bool was_push = false;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( inst_list[i].id == NST_IC_PUSH_VAL )
        {
            was_push = true;
            continue;
        }
        else if ( !was_push ||
            (inst_list[i].id != NST_IC_TYPE_CHECK &&
             inst_list[i].id != NST_IC_HASH_CHECK) ||
            count_jumps_to(bc, i, -1, -1) != 0 )
        {
            was_push = false;
            continue;
        }

        if ( inst_list[i].id == NST_IC_TYPE_CHECK &&
             inst_list[i].val != inst_list[i - 1].val->type )
        {
            Nst_Error error = {
                inst_list[i].start,
                inst_list[i].end,
                NST_E_TYPE_ERROR,
                _nst_format_types_error(
                    EXPECTED_TYPES,
                    AS_STR(inst_list[i].val)->value,
                    TYPE_NAME(inst_list[i - 1].val)
                )
            };

            nst_print_error(error);
            return true;
        }
        else
        {
            nst_hash_obj(inst_list[i - 1].val);
            if ( inst_list[i - 1].val->hash == -1 )
            {
                Nst_Error error = {
                    inst_list[i].start,
                    inst_list[i].end,
                    NST_E_TYPE_ERROR,
                    _nst_format_type_error(
                        UNHASHABLE_TYPE,
                        TYPE_NAME(inst_list[i - 1].val)
                    )
                };

                nst_print_error(error);
                return true;
            }
        }

        inst_list[i].id = NST_IC_NO_OP;
        was_push = false;
    }
    return false;
}

static void remove_push_jumpif(Nst_InstructionList *bc)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;
    register bool expect_jumpif = false;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( inst_list[i].id == NST_IC_PUSH_VAL )
        {
            expect_jumpif = true;
            continue;
        }
        else if ( !expect_jumpif ||
                  !IS_JUMP(inst_list[i].id) ||
                  inst_list[i].id == NST_IC_JUMP ||
                  count_jumps_to(bc, i, -1, -1) != 0 )
        {
            expect_jumpif = false;
            continue;
        }

        if ( inst_list[i].id == NST_IC_JUMPIF_ZERO )
        {
            if ( AS_INT(inst_list[i - 1].val) == 0 )
            {
                inst_list[i - 1].id = NST_IC_NO_OP;
                inst_list[i].id = NST_IC_JUMP;
            }
        }
        else
        {
            Nst_Obj *cond = nst_obj_cast(inst_list[i - 1].val, nst_t_bool, NULL);
            if ( (cond == nst_true && inst_list[i].id == NST_IC_JUMPIF_T) ||
                 (cond == nst_false && inst_list[i].id == NST_IC_JUMPIF_F) )
            {
                nst_dec_ref(inst_list[i - 1].val);
                inst_list[i - 1].id = NST_IC_NO_OP;
                inst_list[i].id = NST_IC_JUMP;
            }
            else
            {
                nst_dec_ref(inst_list[i - 1].val);
                inst_list[i - 1].id = NST_IC_NO_OP;
                inst_list[i].id = NST_IC_NO_OP;
            }
            nst_dec_ref(cond);
        }

        expect_jumpif = false;
    }
}

static void remove_inst(Nst_InstructionList *bc, Nst_Int idx)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;

    if ( idx < 0 || idx >= size )
        return;

    for ( Nst_Int i = idx; i < size - 1; i++ )
        inst_list[i] = inst_list[i + 1];

    --bc->total_size;
    --size;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( IS_JUMP(inst_list[i].id) && inst_list[i].int_val > idx )
            --inst_list[i].int_val;
    }
}

static void optimize_funcs(Nst_InstructionList *bc)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;

    for ( Nst_Int i = 0; i < size - 1; i++ )
    {
        if ( inst_list[i].id == NST_IC_PUSH_VAL && inst_list[i].val->type == nst_t_func )
            nst_optimize_bytecode(AS_FUNC(inst_list[i].val)->body, false);
    }
}

static void remove_dead_code(Nst_InstructionList *bc)
{
    register Nst_Int size = bc->total_size;
    register Nst_RuntimeInstruction *inst_list = bc->instructions;

    for ( Nst_Int i = 0; i < size; i++ )
    {
        if ( inst_list[i].id != NST_IC_JUMP || i + 1 >= inst_list[i].int_val )
            continue;

        bool is_jump_useless = true;
        for ( Nst_Int j = i + 1; j < inst_list[i].int_val; j++ )
        {
            if ( count_jumps_to(bc, j, i + 1, inst_list[i].int_val - 1) != 0 )
            {
                is_jump_useless = false;
                break;
            }

            if ( inst_list[j].val != NULL )
                nst_dec_ref(inst_list[j].val);

            inst_list[j].id = NST_IC_NO_OP;
        }

        if ( is_jump_useless )
            inst_list[i].id = NST_IC_NO_OP;
    }
}
