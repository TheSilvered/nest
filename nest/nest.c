#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "obj.h"

int main()
{
    init_obj();

    Node *ast = parse(ftokenize("_test.nest"));

    if ( ast == NULL )
        return 1;
    
    //print_ast(ast);
    run(ast);
    destroy_node(ast);

    del_obj();

    return 0;
}
