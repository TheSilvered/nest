#include <string.h>
#include "runner.h"
#include "interpreter.h"
#include "mem.h"
#include "lexer.h"
#include "tokens.h"
#include "nodes.h"
#include "parser.h"
#include "compiler.h"
#include "optimizer.h"
#include "function.h"
#include "format.h"
#include "global_consts.h"

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

    if (!Nst_traceback_init(&es->traceback))
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
        Nst_traceback_destroy(&es->traceback);
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

Nst_FuncCall Nst_func_call_from_es(Nst_FuncObj *func, Nst_Pos start,
                                   Nst_Pos end, Nst_ExecutionState *es)
{
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

static Nst_SeqObj *make_argv(i32 argc, i8 **argv, i8 *filename)
{
    Nst_SeqObj *args = SEQ(Nst_array_new(argc + 1));
    if (args == NULL) {
        Nst_error_clear();
        return NULL;
    }

    Nst_Obj *val = Nst_str_new_c_raw(filename, false);
    if (val == NULL) {
        Nst_error_clear();
        args->len = 0;
        Nst_dec_ref(args);
        return NULL;
    }
    Nst_seq_set(args, 0, val);
    Nst_dec_ref(val);

    for (i32 i = 0; i < argc; i++) {
        val = Nst_str_new_c_raw(argv[i], false);
        if (val == NULL) {
            Nst_error_clear();
            args->len = i + 1;
            Nst_dec_ref(args);
            return NULL;
        }
        Nst_seq_set(args, i + 1, val);
        Nst_dec_ref(val);
    }

    return args;
}

bool Nst_es_init_vt(Nst_ExecutionState *es, Nst_CLArgs *cl_args)
{
    i32 argc = cl_args->argc - cl_args->args_start;
    i8 **argv = cl_args->argv + cl_args->args_start;
    i8 *filename = cl_args->filename != NULL ? cl_args->filename : (i8 *)"-c";
    Nst_SeqObj *argv_obj = make_argv(argc, argv, filename);
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
        src->path = (i8 *)"<command>";
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
    Nst_FuncObj *main_func = FUNC(Nst_func_new(0, inst_ls));
    return Nst_run(main_func);
}

void Nst_es_set_cwd(Nst_ExecutionState *es, Nst_StrObj *cwd)
{
    Nst_ndec_ref(es->curr_path);
    es->curr_path = cwd;
}

static Nst_StrObj *make_cwd(i8 *file_path)
{
    i8 *path = NULL;
    i8 *file_part = NULL;

    Nst_get_full_path(file_path, &path, &file_part);
    if (path == NULL) {
        Nst_error_clear();
        return NULL;
    }

    *(file_part - 1) = 0;
    Nst_StrObj *str = STR(Nst_str_new(path, file_part - path - 1, true));
    if (str == NULL) {
        Nst_error_clear();
        Nst_free(file_path);
        return NULL;
    }
    return str;
}

bool Nst_es_push_module(Nst_ExecutionState *es, i8 *filename,
                        Nst_SourceText *source_text)
{
    i32 opt_level = Nst_state.opt_level;
    Nst_InstList *inst_ls = NULL;
    Nst_FuncObj *mod_func = NULL;
    Nst_StrObj *path_str  = NULL;
    Nst_VarTable *vt = NULL;

    i32 file_opt_lvl = 3;
    bool no_default = false;

    Nst_LList *tokens = Nst_tokenizef(
        filename,
        Nst_CP_UNKNOWN,
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

    mod_func = FUNC(Nst_func_new(0, inst_ls));
    if (mod_func == NULL)
        goto cleanup;
    inst_ls = NULL;

    Nst_FuncCall call = Nst_func_call_from_es(
        mod_func,
        Nst_no_pos(),
        Nst_no_pos(),
        es);
    call.cwd = STR(Nst_inc_ref(es->curr_path));

    path_str = make_cwd(filename);
    if (path_str == NULL)
        goto cleanup;

    vt = Nst_vt_new(NULL, es->argv, no_default);
    if (vt == NULL)
        goto cleanup;

    Nst_func_set_vt(mod_func, vt->vars);
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

bool Nst_es_push_func(Nst_ExecutionState *es, Nst_FuncObj *func, Nst_Pos start,
                      Nst_Pos end, i64 arg_num, Nst_Obj **args)
{
    if (func->arg_num < arg_num) {
        Nst_set_call_error(_Nst_EM_WRONG_ARG_NUM_FMT(func->arg_num, arg_num));
        return false;
    }

    Nst_FuncCall call = Nst_func_call_from_es(func, start, end, es);

    Nst_fstack_push(&es->f_stack, call);
    es->idx = 0;

    Nst_VarTable *new_vt = Nst_vt_from_func(func);
    if (new_vt == NULL)
        return false;
    es->vt = new_vt;

    // add the given arguments
    if (args != NULL) {
        for (i64 i = 0; i < arg_num; i++) {
            if (!Nst_vt_set(new_vt, func->args[i], args[i])) {
                Nst_vt_destroy(new_vt);
                return false;
            }
        }
    } else {
        for (i64 i = 0; i < arg_num; i++) {
            Nst_Obj *arg = Nst_vstack_pop(&es->v_stack);
            if (!Nst_vt_set(new_vt, func->args[arg_num - i - 1], arg)) {
                Nst_dec_ref(arg);
                Nst_vt_destroy(new_vt);
                return false;
            }
            Nst_dec_ref(arg);
        }
    }

    // fill the remaining ones with `null`
    for (i64 i = arg_num, n = func->arg_num; i < n; i++) {
        if (!Nst_vt_set(new_vt, func->args[i], Nst_null())) {
            Nst_vt_destroy(new_vt);
            return false;
        }
    }

    Nst_vstack_push(&es->v_stack, NULL);

    return true;
}

bool Nst_es_push_paused_coroutine(Nst_ExecutionState *es, Nst_FuncObj *func,
                                  Nst_Pos start, Nst_Pos end, i64 idx,
                                  Nst_VarTable *vt)
{
    Nst_assert(!Nst_HAS_FLAG(func, Nst_FLAG_FUNC_IS_C));

    Nst_FuncCall call = Nst_func_call_from_es(func, start, end, es);

    if (!Nst_fstack_push(&es->f_stack, call))
        return false;

    es->vt = vt;
    es->idx = idx;
    return true;
}

void Nst_es_force_function_end(Nst_ExecutionState *es)
{
    es->idx = Nst_fstack_peek(&es->f_stack).func->body.bytecode->total_size;
}
