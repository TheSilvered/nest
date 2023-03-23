#include "mem.h"
#include "instructions.h"

Nst_Inst *nst_inst_new(Nst_InstID id, Nst_Pos start, Nst_Pos end, Nst_OpErr *err)
{
    Nst_Inst *inst = (Nst_Inst *)nst_malloc(1, sizeof(Nst_Inst), err);
    if ( inst == NULL )
    {
        return NULL;
    }

    inst->id = id;
    inst->int_val = 0;
    inst->val = NULL;
    inst->start = start;
    inst->end = end;

    return inst;
}

Nst_Inst *_nst_inst_new_val(Nst_InstID id,
                            Nst_Obj   *val,
                            Nst_Pos    start,
                            Nst_Pos    end,
                            Nst_OpErr *err)
{
    Nst_Inst *inst = (Nst_Inst*)nst_malloc(1, sizeof(Nst_Inst), err);
    if ( inst == NULL )
    {
        return NULL;
    }

    inst->id = id;
    inst->int_val = 0;
    inst->val = nst_inc_ref(val);
    inst->start = start;
    inst->end = end;

    return inst;
}

Nst_Inst *nst_inst_new_int(Nst_InstID id,
                           Nst_Int    int_val,
                           Nst_Pos    start,
                           Nst_Pos    end,
                           Nst_OpErr *err)
{
    Nst_Inst *inst = (Nst_Inst *)nst_malloc(1, sizeof(Nst_Inst), err);
    if ( inst == NULL )
    {
        return NULL;
    }

    inst->id = id;
    inst->int_val = int_val;
    inst->val = NULL;
    inst->start = start;
    inst->end = end;

    return inst;
}

void nst_inst_destroy(Nst_Inst *inst)
{
    if ( inst->val != NULL )
    {
        nst_dec_ref(inst->val);
    }
    nst_free(inst);
}

void nst_inst_list_destroy(Nst_InstList *inst_list)
{
    Nst_Inst *instructions = inst_list->instructions;
    for ( Nst_Int i = 0, n = inst_list->total_size; i < n; i++ )
    {
        if ( instructions[i].val != NULL )
        {
            nst_dec_ref(instructions[i].val);
        }
    }

    nst_free(instructions);
    nst_llist_destroy(
        inst_list->functions,
        (Nst_LListDestructor)_nst_dec_ref);
    nst_free(inst_list);
}
