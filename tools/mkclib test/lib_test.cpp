#include "lib_test.h"

#define FUNC_COUNT // Set this to the number of functions in your module

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == NULL )
        return false;

    // Set each function to an index in func_list_
    // func_list_[0] = {
    //     func_ptr, -> the function pointer
    //     1, -> the number of arguments the function takes
    //     new_string_raw("func_name", false) -> the string containing the name
    // }                                           of the funtion inside Nest

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : NULL;
}

// Here you can put the implementations of your functions