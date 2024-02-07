#include "mem.h"
#include "instructions.h"
#include "global_consts.h"

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

Nst_Inst *_Nst_inst_new_val(Nst_InstID id, Nst_Obj *val, Nst_Pos start,
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
    if (inst_ls->instructions == NULL) {
        Nst_free(inst_ls);
        return NULL;
    }
    inst_ls->instructions = inst_array;

    Nst_LList *functions = Nst_llist_new();
    if (inst_ls->functions == NULL) {
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

failure:
    Nst_llist_destroy(functions, (Nst_LListDestructor)_Nst_dec_ref);
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
    Nst_llist_destroy(inst_list->functions, (Nst_LListDestructor)_Nst_dec_ref);
    Nst_free(inst_list);
}
