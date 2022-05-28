#include "lexer.h"
#include "parser.h"

int main()
{
    Node *ast = parse(ftokenize("_test.nest"));

    if ( ast == NULL )
        return 1;

    print_ast(ast);

    return 0;
}
