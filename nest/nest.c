#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "nst_types.h"
#include "obj.h"
#include "map.h"

int main()
{
    Node *ast = parse(ftokenize("_test.nest"));

    if ( ast == NULL )
        return 1;
    //print_ast(ast);
    run(ast);
    destroy_node(ast);

    return 0;
}
