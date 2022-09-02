#include "instructions.h"

Nst_RuntimeInstruction *new_inst_empty(int id, Nst_Int int_val)
{
    Nst_RuntimeInstruction *inst = malloc(sizeof(Nst_RuntimeInstruction));
    if ( inst == NULL )
        return NULL;

    inst->id = id;
    inst->int_val = int_val;
    inst->val = NULL;
    inst->start = nst_no_pos();
    inst->end = nst_no_pos();

    return inst;
}

Nst_RuntimeInstruction *new_inst_pos(int id, Nst_Pos start, Nst_Pos end)
{
    Nst_RuntimeInstruction *inst = malloc(sizeof(Nst_RuntimeInstruction));
    if ( inst == NULL )
        return NULL;

    inst->id = id;
    inst->int_val = 0;
    inst->val = NULL;
    inst->start = start;
    inst->end = end;

    return inst;
}

Nst_RuntimeInstruction *new_inst_val(int id,
                                     Nst_Obj *val,
                                     Nst_Pos start,
                                     Nst_Pos end)
{
    Nst_RuntimeInstruction *inst = malloc(sizeof(Nst_RuntimeInstruction));
    if ( inst == NULL )
        return NULL;

    inst->id = id;
    inst->int_val = 0;
    inst->val = nst_inc_ref(val);
    inst->start = start;
    inst->end = end;

    return inst;
}

Nst_RuntimeInstruction *new_inst_int_val(int id,
                                         Nst_Int int_val,
                                         Nst_Pos start,
                                         Nst_Pos end)
{
    Nst_RuntimeInstruction *inst = malloc(sizeof(Nst_RuntimeInstruction));
    if ( inst == NULL )
        return NULL;

    inst->id = id;
    inst->int_val = int_val;
    inst->val = NULL;
    inst->start = start;
    inst->end = end;

    return inst;
}

void nst_destroy_inst(Nst_RuntimeInstruction *inst)
{
    if ( inst->val != NULL )
        nst_dec_ref(inst->val);
    free(inst);
}

void nst_destroy_inst_list(Nst_InstructionList *inst_list)
{
    Nst_RuntimeInstruction *instructions = inst_list->instructions;
    for ( Nst_Int i = 0, n = inst_list->total_size; i < n; i++ )
    {
        if ( instructions[i].val != NULL )
            nst_dec_ref(instructions[i].val);
    }

    free(instructions);
    free(inst_list);
}
