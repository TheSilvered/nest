#include <string.h>
#include "nest.h"

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

Nst_ExecutionKind Nst_prog_init(Nst_Program *prog, Nst_CLArgs args)
{
    Nst_assert_c(Nst_was_init());

    prog->main_func = NULL;
    prog->argv = NULL;
    prog->source_path = NULL;

    Nst_SourceText *src = Nst_source_load(&args);
    if (src == NULL)
        return Nst_EK_ERROR;

    Nst_DynArray tokens = Nst_tokenize(src);
    if (tokens.len == 0)
        return Nst_EK_ERROR;

    if (args.print_tokens) {
        for (usize i = 0; i < tokens.len; i++) {
            Nst_print_tok(Nst_TOK(Nst_da_get(&tokens, i)));
            Nst_println("");
        }
        Nst_println("");

        if (!args.force_execution && !args.print_ast
            && !args.print_instructions && !args.print_bytecode) {
            Nst_da_clear(&tokens, (Nst_Destructor)Nst_tok_destroy);
            return Nst_EK_INFO;
        }
    }

    Nst_Node *ast = Nst_parse(&tokens);
    Nst_da_clear(&tokens, (Nst_Destructor)Nst_tok_destroy);

    if (args.opt_level >= 1 && ast != NULL)
        ast = Nst_optimize_ast(ast);

    if (ast == NULL)
        return Nst_EK_ERROR;

    if (args.print_ast) {
        Nst_print_node(ast);
        Nst_println("");

        if (!args.force_execution && !args.print_instructions
            && !args.print_bytecode) {
            Nst_node_destroy(ast);
            return Nst_EK_INFO;
        }
    }

    Nst_InstList inst_ls = Nst_compile(ast, false);
    Nst_node_destroy(ast);
    if (Nst_ilist_len(&inst_ls) == 0)
        return Nst_EK_ERROR;

    if (args.opt_level >= 2 && Nst_ilist_len(&inst_ls) != 0) {
        bool optimize_builtins = args.opt_level == 3 && !args.no_default;
        Nst_optimize_ilist(&inst_ls, optimize_builtins);
    }

    if (args.print_instructions) {
        Nst_ilist_print(&inst_ls);
        Nst_println("");

        if (!args.force_execution && !args.print_bytecode) {
            Nst_ilist_destroy(&inst_ls);
            return Nst_EK_INFO;
        }
    }

    Nst_Bytecode *bc = Nst_assemble(&inst_ls);
    Nst_ilist_destroy(&inst_ls);
    if (bc == NULL)
        return Nst_EK_ERROR;
    if (args.print_bytecode) {
        Nst_bc_print(bc);
        Nst_println("");

        if (!args.force_execution) {
            Nst_bc_destroy(bc);
            return Nst_EK_INFO;
        }
    }
    Nst_Obj *main_func = _Nst_func_new(NULL, 0, bc);
    if (main_func == NULL) {
        Nst_bc_destroy(bc);
        return Nst_EK_ERROR;
    }
    prog->main_func = main_func;

    prog->argv = make_argv(
        args.argc, args.argv,
        args.filename != NULL ? args.filename : "-c");
    if (prog->argv == NULL) {
        Nst_prog_destroy(prog);
        return Nst_EK_ERROR;
    }
    prog->source_path = Nst_str_new_c_raw(src->path, false);
    if (prog->source_path == NULL) {
        Nst_prog_destroy(prog);
        return Nst_EK_ERROR;
    }

    Nst_VarTable prog_vt;
    bool success = Nst_vt_init(&prog_vt, NULL, prog->argv, args.no_default);
    if (!success) {
        Nst_prog_destroy(prog);
        return false;
    }
    _Nst_func_set_mod_globals(prog->main_func, prog_vt.vars);
    Nst_dec_ref(prog_vt.vars);

    return Nst_EK_RUN;
}

void Nst_prog_destroy(Nst_Program *prog)
{
    Nst_ndec_ref(prog->main_func);
    Nst_ndec_ref(prog->argv);
    Nst_ndec_ref(prog->source_path);
}
