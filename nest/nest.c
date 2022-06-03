#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "nst_types.h"
#include "obj.h"
#include "map.h"

int main()
{
    /*init_obj();

    Nst_map *map = new_map();

    Nst_Obj *key = NULL;
    Nst_Obj *val = NULL;

    for ( int i = 0; i < 100; i++ )
    {
        key = make_obj(new_int(i), nst_t_int, free);
        val = make_obj(new_int(i * 10), nst_t_int, free);
        map_set(map, key, val);
    }

    for ( int i = 0; i < map->size; i++ )
    {
        if ( map->nodes[i].key == NULL )
            continue;
        printf("%lli: %lli\n", *AS_INT(map->nodes[i].key->value), *AS_INT(map->nodes[i].value->value));
    }

    printf("%lli\n", *AS_INT(map_get(map, make_obj(new_int(10), nst_t_int, free))->value));*/

    Node *ast = parse(ftokenize("_test.nest"));

    if ( ast == NULL )
        return 1;

    run(ast);

    return 0;
}
