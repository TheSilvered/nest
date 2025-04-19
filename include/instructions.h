/**
 * @file instructions.h
 *
 * @brief Bytecode instruction interface
 *
 * @author TheSilvered
 */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "simple_types.h"
#include "error.h"
#include "llist.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef enum _Nst_InstCode {
    Nst_IC_NO_OP,
    Nst_IC_POP_VAL,
    Nst_IC_FOR_START,
    Nst_IC_FOR_NEXT,
    Nst_IC_RETURN_VAL,
    Nst_IC_RETURN_VARS,
    Nst_IC_SET_VAL_LOC,
    Nst_IC_SET_CONT_LOC,
    Nst_IC_THROW_ERR,
    Nst_IC_POP_CATCH,
    Nst_IC_SET_VAL,
    Nst_IC_GET_VAL,
    Nst_IC_PUSH_VAL,
    Nst_IC_SET_CONT_VAL,
    Nst_IC_OP_CALL,
    Nst_IC_OP_SEQ_CALL,
    Nst_IC_OP_CAST,
    Nst_IC_OP_RANGE,
    Nst_IC_STACK_OP,
    Nst_IC_LOCAL_OP,
    Nst_IC_OP_IMPORT,
    Nst_IC_OP_EXTRACT,
    Nst_IC_DEC_INT,
    Nst_IC_NEW_INT,
    Nst_IC_DUP,
    Nst_IC_ROT_2,
    Nst_IC_ROT_3,
    Nst_IC_MAKE_ARR,
    Nst_IC_MAKE_ARR_REP,
    Nst_IC_MAKE_VEC,
    Nst_IC_MAKE_VEC_REP,
    Nst_IC_MAKE_MAP,
    Nst_IC_MAKE_FUNC,
    Nst_IC_SAVE_ERROR,
    Nst_IC_UNPACK_SEQ,
    Nst_IC_JUMP,
    Nst_IC_JUMPIF_T,
    Nst_IC_JUMPIF_F,
    Nst_IC_JUMPIF_ZERO,
    Nst_IC_JUMPIF_IEND,
    Nst_IC_PUSH_CATCH
} Nst_InstCode;

NstEXP typedef struct _Nst_Inst {
    Nst_InstCode code;
    Nst_Span span;
    i64 val;
} Nst_Inst;

NstEXP typedef struct _Nst_InstList {
    Nst_DynArray instructions;
    Nst_DynArray objects;
    Nst_DynArray functions;
} Nst_InstList;

NstEXP typedef struct _Nst_FuncPrototype {
    Nst_InstList ilist;
    Nst_ObjRef **arg_names;
    usize arg_num;
} Nst_FuncPrototype;

NstEXP bool NstC Nst_ic_is_jump(Nst_InstCode code);

NstEXP bool NstC Nst_ilist_init(Nst_InstList *list);
NstEXP void NstC Nst_ilist_destroy(Nst_InstList *list);

NstEXP bool NstC Nst_ilist_add(Nst_InstList *list, Nst_InstCode code,
                               Nst_Span span);

NstEXP bool NstC Nst_ilist_add_ex(Nst_InstList *list, Nst_InstCode code,
                                  i64 val, Nst_Span span);

NstEXP isize NstC Nst_ilist_add_obj(Nst_InstList *list, Nst_ObjRef *obj);
NstEXP isize NstC Nst_ilist_add_func(Nst_InstList *list,
                                     Nst_FuncPrototype *fp);

NstEXP Nst_Inst *NstC Nst_ilist_get_inst(Nst_InstList *list, usize idx);
NstEXP Nst_Obj *NstC Nst_ilist_get_inst_obj(Nst_InstList *list, usize idx);
NstEXP Nst_FuncPrototype *NstC Nst_ilist_get_inst_func(Nst_InstList *list,
                                                       usize idx);
NstEXP Nst_Obj *NstC Nst_ilist_get_obj(Nst_InstList *list, usize idx);
NstEXP Nst_FuncPrototype *NstC Nst_ilist_get_func(Nst_InstList *list,
                                                  usize idx);

NstEXP void NstC Nst_ilist_set(Nst_InstList *list, usize idx,
                               Nst_InstCode code);
NstEXP void NstC Nst_ilist_set_ex(Nst_InstList *list, usize idx,
                                  Nst_InstCode code, i64 val);
NstEXP usize NstC Nst_ilist_len(Nst_InstList *list);

NstEXP bool NstC Nst_fprototype_init(Nst_FuncPrototype *fp, Nst_InstList ls,
                                     usize arg_num);
NstEXP void NstC Nst_fprototype_destroy(Nst_FuncPrototype *fp);

NstEXP void NstC Nst_ilist_print(Nst_InstList *list);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // INSTRUCTIONS_H
