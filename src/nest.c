#include <string.h>
#include <stdlib.h>
#include "nest.h"

#ifdef _EXIT
#undef _EXIT
#endif

#define _EXIT(code) do {                                                      \
    _Nst_del_objects();                                                       \
    _Nst_unload_libs();                                                       \
    if (cl_args.filename != NULL) {                                           \
        Nst_free(src_text.text);                                              \
        Nst_free(src_text.lines);                                             \
        Nst_free(src_text.path);                                              \
    }                                                                         \
    return code;                                                              \
    } while (0)

#define ERROR_EXIT                                                            \
    do {                                                                      \
    Nst_print_error(error);                                                   \
    Nst_dec_ref(error.name);                                                  \
    Nst_dec_ref(error.message);                                               \
    EXIT(1);                                                                  \
    } while (0)

#ifdef Nst_WIN

#include <windows.h>

#define EXIT(code) do {                                                       \
        Nst_free(argv);                                                       \
        _EXIT(code);                                                          \
    } while (0)

int wmain(int argc, wchar_t **wargv)
{
    _Nst_set_console_mode();

    char **argv;

    if (!_Nst_wargv_to_argv(argc, wargv, &argv))
        return -1;

#else

#define EXIT _EXIT

int main(int argc, char **argv)
{

#endif

#ifdef _DEBUG
    puts("**USING DEBUG BUILD - " Nst_VERSION "**");
    for (usize i = 0, n = strlen(Nst_VERSION) + 24; i < n; i++)
        putc('-', stdout);
    putc('\n', stdout);
    fflush(stdout);
#endif

    Nst_CLArgs cl_args;
    i32 parse_result = _Nst_parse_args(argc, argv, &cl_args);

    Nst_set_color(Nst_supports_color());

    if (parse_result == -1)
        return -1;
    else if (parse_result == 1)
        return 0;

    if (!_Nst_init_objects()) {
#ifdef Nst_WIN
        Nst_free(argv);
#endif
        printf("Failed allocation\n");
        return -1;
    }
    Nst_ggc_init();

    Nst_LList *tokens;
    Nst_Error error = { false, Nst_no_pos(), Nst_no_pos(), NULL, NULL };
    Nst_SourceText src_text = { NULL, NULL, NULL, 0, 0 };

    if (cl_args.filename != NULL) {
        i32 spec_opt_lvl;
        bool spec_no_def;
        tokens = Nst_tokenizef(
            cl_args.filename,
            cl_args.encoding,
            &spec_opt_lvl,
            &spec_no_def,
            &src_text,
            &error);

        if (spec_opt_lvl < cl_args.opt_level)
            cl_args.opt_level = spec_opt_lvl;
        if (spec_no_def)
            cl_args.no_default = true;
    } else {
        src_text.path = (i8 *)"<command>";
        src_text.text_len = strlen(cl_args.command);
        src_text.text = cl_args.command;
        src_text.lines_len = 1;
        src_text.lines = &src_text.text;
        tokens = Nst_tokenize(&src_text, &error);
    }

    if (tokens == NULL) {
        if (src_text.text == NULL)
            EXIT(0);
        else
            ERROR_EXIT;
    }

    if (cl_args.print_tokens) {
        for (Nst_LLNode *n = tokens->head; n != NULL; n = n->next) {
            Nst_print_tok(Nst_TOK(n->value));
            printf("\n");
        }

        if (!cl_args.force_execution
            && !cl_args.print_ast
            && !cl_args.print_bytecode)
        {
            Nst_llist_destroy(tokens, (Nst_LListDestructor)Nst_tok_destroy);
            EXIT(0);
        }
    }

    Nst_Node *ast = Nst_parse(tokens, &error);

    if (cl_args.opt_level >= 1 && ast != NULL)
        ast = Nst_optimize_ast(ast, &error);

    // Nst_optimize_ast can delete the ast
    if (ast == NULL)
        ERROR_EXIT;

    if (cl_args.print_ast) {
        if (cl_args.print_tokens)
            printf("\n");
        Nst_print_ast(ast);

        if (!cl_args.force_execution && !cl_args.print_bytecode) {
            Nst_node_destroy(ast);
            EXIT(0);
        }
    }

    // nst_compile never fails
    Nst_InstList *inst_ls = Nst_compile(ast, false, &error);

    if (cl_args.opt_level >= 2 && inst_ls != NULL) {
        bool optimize_builtins = cl_args.opt_level == 3 && !cl_args.no_default;
        inst_ls = Nst_optimize_bytecode(inst_ls, optimize_builtins, &error);
    }
    if (inst_ls == NULL) {
        _Nst_ggc_delete_objs();
        ERROR_EXIT;
    }

    if (cl_args.print_bytecode) {
        if (cl_args.print_tokens || cl_args.print_ast)
            printf("\n");
        Nst_print_bytecode(inst_ls);

        if (!cl_args.force_execution) {
            Nst_inst_list_destroy(inst_ls);
            EXIT(0);
        }
    }

    if (!Nst_state_init(
        argc - cl_args.args_start,
        argv + cl_args.args_start,
        cl_args.filename,
        cl_args.opt_level,
        cl_args.no_default))
    {
        fprintf(stderr, "Failed allocation\n");
        EXIT(-1);
    }

    Nst_FuncObj *main_func = FUNC(Nst_func_new(0, inst_ls));

    i32 exe_result = Nst_run(main_func);

    EXIT(exe_result);
}
