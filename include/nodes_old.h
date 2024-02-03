/* [docs:ignore_file] */
#if 0
/**
 * @file nodes.h
 *
 * @brief Nodes of the AST
 *
 * @author TheSilvered
 */

#ifndef NODES_OLD_H
#define NODES_OLD_H

#include "error.h"

/* Casts `expr` to `Nst_Node *`. */
#define Nst_NODE(expr) ((Nst_Node__old *)(expr))
/* Evaluates to `true` if the specified node type returns a value. */
#define Nst_NODE_RETUNS_VALUE(node_type) (node_type >= Nst_NT_STACK_OP)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* The types of nodes in the AST. */
/*
NstEXP typedef enum _Nst_NodeType__old {
    Nst_NT_LONG_S,
    Nst_NT_WHILE_L,
    Nst_NT_DOWHILE_L,
    Nst_NT_FOR_L,
    Nst_NT_FOR_AS_L,
    Nst_NT_FUNC_DECLR,
    Nst_NT_RETURN_S,
    Nst_NT_CONTINUE_S,
    Nst_NT_BREAK_S,
    Nst_NT_SWITCH_S,
    Nst_NT_TRY_CATCH_S,

    // These nodes return a value

    Nst_NT_STACK_OP,
    Nst_NT_LOCAL_STACK_OP,
    Nst_NT_LOCAL_OP,
    Nst_NT_ARR_LIT,
    Nst_NT_VEC_LIT,
    Nst_NT_MAP_LIT,
    Nst_NT_VALUE,
    Nst_NT_ACCESS,
    Nst_NT_EXTRACT_E,
    Nst_NT_ASSIGN_E,
    Nst_NT_IF_E,
    Nst_NT_LAMBDA
} Nst_NodeType__old;
*/
/**
 * A structure representing a node.
 *
 * @param start: the starting position of the node
 * @param end: the ending position of the node
 * @param type: the type of the node
 * @param nodes: the child nodes of the node
 * @param tokens: the tokens that the node contains
 */
NstEXP typedef struct _Nst_Node__old {
    Nst_Pos start;
    Nst_Pos end;
    i32 type;
    Nst_LList *nodes;
    Nst_LList *tokens;
} Nst_Node__old;

/**
 * Creates and initializes a new AST node on the heap.
 *
 * @brief Though the position is not in the arguments, its position should be
 * set later with `Nst_node_set_pos`.
 *
 * @param type: the type of the node to initialize
 *
 * @return The new node or `NULL` on failure. The error is set.
 */
NstEXP Nst_Node__old *NstC Nst_node_new__old(i32 type);
/**
 * Creates and initializes a new AST node on the heap.
 *
 * @param type: the type of the node to initialize
 * @param start: the start position of the node
 * @param end: the end position of the node
 *
 * @return The new node or `NULL` on failure. The error is set.
 */
NstEXP Nst_Node__old *NstC Nst_node_new_pos__old(i32 type, Nst_Pos start,
                                       Nst_Pos end);
/* Sets the position of a node. */
NstEXP void NstC Nst_node_set_pos__old(Nst_Node__old *node, Nst_Pos start, Nst_Pos end);
/* Destroys a node and its children. */
NstEXP void NstC Nst_node_destroy__old(Nst_Node__old *node);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NODES_OLD_H
#endif
;
