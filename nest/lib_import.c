#include <stdlib.h>
#include "lib_import.h"

FuncDeclr *new_func_list(size_t count)
{
    return (FuncDeclr *)malloc((count + 1) * sizeof(FuncDeclr));
}
