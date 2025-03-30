#include <string.h>
#include "nest.h"

bool Nst_es_init(Nst_ExecutionState *es)
{
    Nst_assert_c(Nst_was_init());

    es->idx = 0;
    es->argv = NULL;
    es->vt = NULL;
    es->source_path = NULL;
    es->curr_path = Nst_getcwd();

    if (es->curr_path == NULL)
        goto cleanup;

    if (!Nst_tb_init(&es->traceback))
        goto cleanup;

    if (!Nst_vstack_init(&es->v_stack))
        goto cleanup;

    if (!Nst_fstack_init(&es->f_stack))
        goto cleanup;

    if (!Nst_cstack_init(&es->c_stack))
        goto cleanup;

    return true;

cleanup:
    Nst_es_destroy(es);
    return false;
}

void Nst_es_destroy(Nst_ExecutionState *es)
{
    if (es->traceback.positions != NULL)
        Nst_tb_destroy(&es->traceback);
    if (es->v_stack.stack != NULL)
        Nst_vstack_destroy(&es->v_stack);
    if (es->f_stack.stack != NULL)
        Nst_fstack_destroy(&es->f_stack);
    if (es->c_stack.stack != NULL)
        Nst_cstack_destroy(&es->c_stack);

    Nst_ndec_ref(es->curr_path);
    Nst_ndec_ref(es->source_path);
    Nst_ndec_ref(es->argv);
    if (es->vt != NULL)
        Nst_vt_destroy(es->vt);
}

Nst_FuncCall Nst_func_call_from_es(Nst_Obj *func, Nst_Pos start,
                                   Nst_Pos end, Nst_ExecutionState *es)
{
    Nst_assert(func->type == Nst_t.Func);
    Nst_FuncCall call = {
        .func = func,
        .start = start,
        .end = end,
        .vt = es->vt,
        .idx = es->idx,
        .cwd = NULL,
        .cstack_len = es->c_stack.len
    };
    return call;
}

static Nst_Obj *make_argv(i32 argc, char **argv, char *filename)
{
    Nst_Obj *args = Nst_array_new(argc + 1);
    if (args == NULL) {
        Nst_error_clear();
        return NULL;
    }

    Nst_Obj *val = Nst_str_new_c_raw(filename, false);
    if (val == NULL) {
        Nst_error_clear();
        Nst_dec_ref(args);
        return NULL;
    }
    Nst_seq_setn(args, 0, val);

    for (i32 i = 0; i < argc; i++) {
        val = Nst_str_new_c_raw(argv[i], false);
        if (val == NULL) {
            Nst_error_clear();
            Nst_dec_ref(args);
            return NULL;
        }
        Nst_seq_setn(args, i + 1, val);
    }

    return args;
}

bool Nst_es_init_vt(Nst_ExecutionState *es, Nst_CLArgs *cl_args)
{
    i32 argc = cl_args->argc - cl_args->args_start;
    char **argv = cl_args->argv + cl_args->args_start;
    char *filename = cl_args->filename != NULL ? cl_args->filename : "-c";
    Nst_Obj *argv_obj = make_argv(argc, argv, filename);
    if (argv_obj == NULL)
        return false;
    es->argv = argv_obj;
    Nst_VarTable *vt = Nst_vt_new(
        NULL,
        es->argv,
        cl_args->no_default);
    if (vt == NULL)
        return false;
    es->vt = vt;

    if (filename == NULL) {
        es->source_path = NULL;
        return true;
    }

    return true;
}

i32 Nst_execute(Nst_CLArgs args, Nst_ExecutionState *es, Nst_SourceText *src)
{
    Nst_state_set_es(NULL);
    Nst_es_init(es);
    Nst_state_set_es(es);
    Nst_source_text_init(src);

    Nst_LList *tokens;
    args.opt_level = Nst_state.opt_level;

    if (args.filename != NULL) {
        tokens = Nst_tokenizef(
            args.filename,
            args.encoding,
            &args.opt_level,
            &args.no_default,
            src);
    } else {
        src->path = "<command>";
        src->text_len = strlen(args.command);
        src->text = args.command;
        src->lines_len = 1;
        src->lines = &src->text;
        src->allocated = false;
        tokens = Nst_tokenize(src);
    }

    if (tokens == NULL)
        return 1;

    if (args.print_tokens) {
        for (Nst_LLNode *n = tokens->head; n != NULL; n = n->next) {
            Nst_print_tok(Nst_TOK(n->value));
            printf("\n");
        }

        if (!args.force_execution && !args.print_ast && !args.print_bytecode) {
            Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
            return 0;
        }
    }

    Nst_Node *ast = Nst_parse(tokens);

    if (args.opt_level >= 1 && ast != NULL)
        ast = Nst_optimize_ast(ast);

    if (ast == NULL)
        return 1;

    if (args.print_ast) {
        if (args.print_tokens)
            printf("\n");
        Nst_print_node(ast);

        if (!args.force_execution && !args.print_bytecode) {
            Nst_node_destroy(ast);
            return 0;
        }
    }

    Nst_InstList *inst_ls = Nst_compile(ast, false);

    if (args.opt_level >= 2 && inst_ls != NULL) {
        bool optimize_builtins = args.opt_level == 3 && !args.no_default;
        inst_ls = Nst_optimize_bytecode(inst_ls, optimize_builtins);
    }

    if (inst_ls == NULL)
        return 1;

    if (args.print_bytecode) {
        if (args.print_tokens || args.print_ast)
            printf("\n");
        Nst_inst_list_print(inst_ls);

        if (!args.force_execution) {
            Nst_inst_list_destroy(inst_ls);
            return 0;
        }
    }

    Nst_es_init_vt(es, &args);
    Nst_Obj *main_func = _Nst_func_new(0, inst_ls);
    return Nst_run(main_func);
}

void Nst_es_set_cwd(Nst_ExecutionState *es, Nst_Obj *cwd)
{
    Nst_ndec_ref(es->curr_path);
    es->curr_path = cwd;
}

static Nst_Obj *make_cwd(const char *file_path)
{
    char *path = NULL;
    char *file_part = NULL;

    Nst_get_full_path(file_path, &path, &file_part);
    if (path == NULL) {
        Nst_error_clear();
        return NULL;
    }

    *(file_part - 1) = 0;
    Nst_Obj *str = Nst_str_new((u8 *)path, file_part - path - 1, true);
    if (str == NULL) {
        Nst_error_clear();
        Nst_free(path);
        return NULL;
    }
    return str;
}

bool Nst_es_push_module(Nst_ExecutionState *es, const char *filename,
                        Nst_SourceText *source_text)
{
    i32 opt_level = Nst_state.opt_level;
    Nst_InstList *inst_ls = NULL;
    Nst_Obj *mod_func = NULL;
    Nst_Obj *path_str  = NULL;
    Nst_VarTable *vt = NULL;

    i32 file_opt_lvl = 3;
    bool no_default = false;

    Nst_LList *tokens = Nst_tokenizef(
        filename,
        Nst_EID_UNKNOWN,
        &file_opt_lvl,
        &no_default,
        source_text);

    if (file_opt_lvl < opt_level)
        opt_level = file_opt_lvl;

    if (tokens == NULL)
        goto cleanup;

    Nst_Node *ast = Nst_parse(tokens);
    if (ast != NULL && opt_level >= 1)
        ast = Nst_optimize_ast(ast);

    if (ast == NULL)
        goto cleanup;

    inst_ls = Nst_compile(ast, true);
    if (opt_level >= 2 && inst_ls != NULL) {
        inst_ls = Nst_optimize_bytecode(
            inst_ls,
            opt_level == 3 && !no_default);
    }
    if (inst_ls == NULL)
        goto cleanup;

    mod_func = _Nst_func_new(0, inst_ls);
    if (mod_func == NULL)
        goto cleanup;
    inst_ls = NULL;

    Nst_FuncCall call = Nst_func_call_from_es(
        mod_func,
        Nst_pos_empty(),
        Nst_pos_empty(),
        es);
    call.cwd = Nst_inc_ref(es->curr_path);

    path_str = make_cwd(filename);
    if (path_str == NULL)
        goto cleanup;

    vt = Nst_vt_new(NULL, es->argv, no_default);
    if (vt == NULL)
        goto cleanup;

    _Nst_func_set_mod_globals(mod_func, vt->vars);
    Nst_es_set_cwd(es, path_str);
    path_str = NULL; // the reference is taken by the execution state
    es->vt = vt;
    es->idx = 0;
    if (!Nst_fstack_push(&es->f_stack, call))
        goto cleanup;

    Nst_dec_ref(mod_func);
    mod_func = NULL;

    return Nst_vstack_push(&es->v_stack, NULL);

cleanup:
    if (inst_ls != NULL)
        Nst_inst_list_destroy(inst_ls);
    Nst_ndec_ref(mod_func);
    Nst_ndec_ref(path_str);
    return false;
}

bool Nst_es_push_func(Nst_ExecutionState *es, Nst_Obj *func, Nst_Pos start,
                      Nst_Pos end, usize arg_num, Nst_Obj **args)
{
    usize func_arg_num = Nst_func_arg_num(func);
    if (func_arg_num < arg_num) {
        Nst_error_set_call(_Nst_WRONG_ARG_NUM(func_arg_num, arg_num));
        return false;
    }

    Nst_FuncCall call = Nst_func_call_from_es(func, start, end, es);

    Nst_fstack_push(&es->f_stack, call);
    es->idx = 0;

    Nst_VarTable *new_vt;

    Nst_Obj *func_globals = Nst_func_mod_globals(func);
    if (func_globals != NULL)
        new_vt = Nst_vt_new(func_globals, NULL, false);
    else if (Nst_state.es->vt->global_table == NULL)
        new_vt = Nst_vt_new(Nst_state.es->vt->vars, NULL, false);
    else
        new_vt = Nst_vt_new(Nst_state.es->vt->global_table, NULL, false);

    if (new_vt == NULL)
        return false;
    es->vt = new_vt;

    // add the outer variables if needed
    Nst_Obj *outer_vars = Nst_func_outer_vars(func);
    if (outer_vars != NULL) {
        Nst_Obj *key;
        Nst_Obj *val;
        for (isize i = Nst_map_next(-1, outer_vars, &key, &val);
             i != -1;
             i = Nst_map_next(i, outer_vars, &key, &val))
        {
            if (!Nst_vt_set(new_vt, key, val)) {
                Nst_vt_destroy(new_vt);
                return false;
            }
        }
    }

    // add the given arguments
    Nst_Obj **func_args = Nst_func_args(func);
    if (args != NULL) {
        for (usize i = 0; i < arg_num; i++) {
            if (!Nst_vt_set(new_vt, func_args[i], args[i])) {
                Nst_vt_destroy(new_vt);
                return false;
            }
        }
    } else {
        for (usize i = 0; i < arg_num; i++) {
            Nst_Obj *arg = Nst_vstack_pop(&es->v_stack);
            if (!Nst_vt_set(new_vt, func_args[arg_num - i - 1], arg)) {
                Nst_dec_ref(arg);
                Nst_vt_destroy(new_vt);
                return false;
            }
            Nst_dec_ref(arg);
        }
    }

    // fill the remaining ones with `null`
    for (usize i = arg_num; i < func_arg_num; i++) {
        if (!Nst_vt_set(new_vt, func_args[i], Nst_null())) {
            Nst_vt_destroy(new_vt);
            return false;
        }
    }

    Nst_vstack_push(&es->v_stack, NULL);

    return true;
}

bool Nst_es_push_paused_coroutine(Nst_ExecutionState *es, Nst_Obj *func,
                                  Nst_Pos start, Nst_Pos end, i64 idx,
                                  Nst_VarTable *vt)
{
    Nst_assert(func->type == Nst_t.Func);
    Nst_assert(!Nst_FUNC_IS_C(func));

    Nst_FuncCall call = Nst_func_call_from_es(func, start, end, es);

    if (!Nst_fstack_push(&es->f_stack, call))
        return false;

    es->vt = vt;
    es->idx = idx;
    return true;
}

void Nst_es_force_function_end(Nst_ExecutionState *es)
{
    es->idx =
        Nst_func_nest_body(Nst_fstack_peek(&es->f_stack).func)->total_size;
}
