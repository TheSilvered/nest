import sys
import os
import shutil
import string

# not all source files, only the one that can be used
nest_source_files = [
    "error.h",
    "function.h",
    "hash.h",
    "iter.h",
    "lib_import.h",
    "llist.h",
    "map.h",
    "nest_include.h",
    "nodes.h",
    "nst_types.h",
    "obj.h",
    "obj_ops.h",
    "sequence.h",
    "simple_types.h",
    "str.h",
    "tokens.h",
    "var_table.h",
    "error.c",
    "function.c",
    "hash.c",
    "iter.c",
    "lib_import.c",
    "llist.c",
    "map.c",
    "nodes.c",
    "obj.c",
    "obj_ops.c",
    "sequence.c",
    "simple_types.c",
    "str.c",
    "tokens.c",
    "var_table.c"
]


def main():
    if len(sys.argv) not in (3, 4):
        print("USAGE: py .\\make_clib_files.py (new LIBRARY_NAME|update) LOCATION")
        exit(1)

    operation = sys.argv[1].strip()
    project_name = sys.argv[2].strip() if len(sys.argv) == 4 else None
    location = sys.argv[3 if len(sys.argv) == 4 else 2].strip()

    if operation not in ("new", "update"):
        print(f"The given operation (\"{operation}\") is not valid")
        exit(1)

    if not os.path.isdir(location):
        print(f"The given path (\"{location}\") doesn't exist")
        exit(2)

    if not os.path.isdir(f"{location}\\nest_source"):
            os.mkdir(f"{location}\\nest_source")

    for file_name in nest_source_files:
        shutil.copyfile(
            f"..\\nest\\{file_name}",
            f"{location}\\nest_source\\{file_name}"
        )

    if operation == "update":
        exit(0)

    project_name_c = project_name.replace(" ", "_").replace("\t", "_")

    if set(project_name_c) & set(string.digits + string.ascii_letters + "_") != set(project_name_c):
        print(f"The given name (\"{project_name}\") is not valid")
        exit(3)

    with open(f"{location}\\{project_name}.h", "w") as header_file:
        header_file.write(f"""#ifndef {project_name_c.upper()}_H
#define {project_name_c.upper()}_H

#include "nest_source/nest_include.h"

#ifdef {project_name_c.upper().replace("_", "")}_EXPORTS
#define {project_name_c.upper()}_API __declspec(dllexport)
#else
#define {project_name_c.upper()}_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {{
#endif // !__cplusplus

{project_name_c.upper()}_API bool lib_init();
{project_name_c.upper()}_API FuncDeclr *get_func_ptrs();
{project_name_c.upper()}_API INIT_LIB_OBJ_FUNC;

// Here you can put your function signatures
// They must always be `Nst_Obj *func_name(size_t arg_num, Nst_Obj **args, OpErr *err);`
// replace func_name with your function's name

// To compile remove precompiled headers in Project->Properties->C/C++->
// ->Precompiled headers and set "Precompiled header" to "Not Using Precompiled
// Headers"

#ifdef __cplusplus
}}
#endif // !__cplusplus

#endif // !{project_name_c.upper()}_H
""")
        with open(f"{location}\\{project_name}.cpp", "w") as source_file:
            source_file.write(f"""#include "{project_name}.h"

#define FUNC_COUNT // Set this to the number of functions in your module

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == NULL )
        return false;

    // Set each function to an index in func_list_
    // func_list_[0] = {{
    //     func_ptr, -> the function pointer
    //     1, -> the number of arguments the function takes
    //     new_string_raw("func_name", false) -> the string containing the name
    // }}                                        of the funtion inside Nest

    lib_init_ = true;
    return true;
}}

FuncDeclr *get_func_ptrs()
{{
    return lib_init_ ? func_list_ : NULL;
}}

// Here you can put the implementations of your functions
""")

        exit(0)


if __name__ == "__main__":
    main()
