# Creating your own C library

## Installation of the source

First things first you need to clone the latest stable release of Nest by
executing:

```text
git clone -b beta-0.12.1 https://github.com/TheSilvered/nest.git
cd nest
```

Then you can compile it from source as specified in the
[installation](../installation.md) documentation.

Here is a little explanation of what all the folders contain:

```text
nest
├───build # various files to compile Nest and the installers
├───docs # the documentation
├───examples # some examples of Nest code
├───include # the header files for the core interpreter
├───libs # the standard libraries
├───plugins # the plugins for SublimeText and VSCode
├───src # the source of the core interpreter
├───tests # the tests for the libraries and the interpreter
└───tools # tools for developing Nest
```

From here navigate to `tools/` where you will find `mkclib.nest`. This is a
utility that will generate the necessary files to create your own C library.

## `mkclib.nest`

Running `mkclib.nest` without any arguments will display this:

```text
$ nest mkclib.nest
Usage: nest mkclib.nest [new <name>|update] <path>
```

This means that there are two modes to use this utility: the first (`new`)
creates a new library whereas `update` will update the headers and the shared
objects.

As of now, we are only interested in `new`, so let's go ahead and create a new
library called `my_lib` and put it in `~/my_lib/`.

```text
$ nest mkclib.nest new my_lib ~/
Copied headers
Updated Windows x86 Debug
Updated Windows x86 Release
Updated Windows x64 Debug
Updated Windows x64 Release
Updated Linux x64/x86 Debug
Updated Linux x86 Release
Updated Linux x64 Release
Created 'my_lib.sln'
Created 'makefile'
Created 'my_lib.nest'
Created 'my_lib.vcxproj'
Created 'my_lib.vcxproj.filters'
Created 'dllmain.cpp'
Created 'framework.h'
Created 'my_lib.h'
Created 'my_lib.cpp'
'my_lib' created succesfully!
```

You may also see a message such as this:

```text
'../build/windows/projects/nest/x64/Release/libnest.lib' could not be found
'../build/windows/projects/nest/x64/Release/libnest.dll' could not be found
  To add the file(s) build Windows x64 Release and update the library
```

But not to worry! This depends on what configuration you have compiled, the
library will still be created, just keep in mind that if you want to compile it
for that configuration as well, you need to compile Nest for it first.

Now that the library has been created you can navigate to `~/my_lib/my_lib`
to access the source files. `dllmain.cpp` and `framework.h` are used on Windows
to generate the DLL and `my_lib.vcxproj` and `my_lib.vcxproj.filters` are
project files used by Visual Studio.  
Furthermore, if you navigate to `~/my_lib/nest_source` you will find all the
necessary headers.

## Analyzing the generated files

The other two files inside `~/my_lib/my_lib` are `my_lib.h` and `my_lib.cpp`
that are the ones that you will want to modify.

`my_lib.h` contains the following:

```better-c
#ifndef MY_LIB_H
#define MY_LIB_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT bool lib_init();
EXPORT Nst_DeclrList *get_func_ptrs();
// EXPORT void free_lib();

NST_FUNC_SIGN(add);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MY_LIB_H
```

Here, excluding boilerplate code, you will see two three function declarations,
the third of which is commented out, and another line to which we'll get to in
a minute. These functions are the means that the main interpreter uses to
communicate with the library.

- `lib_init` is the function that is called when the library is first loaded and
  should initialize the library
- `get_func_ptrs` will give to the interpreter all the function and object
  declarations that the user of the library can access
- `free_lib` is optional, as you can see here, and is called when the main
  interpreter is closing and the library is unloaded, here there should be the
  cleanup of the things initialized with `lib_init`

Now we can get to the last line: `NST_FUNC_SIGN(add)`. This is also a function
declaration that would appear like this without the macro:

```better-c
Nst_Obj *add(usize arg_num, Nst_Obj **args, Nst_OpErr *err);
```

Though using the macro is preferred since other macros rely on the correct
naming of the arguments.

This kind of function is what can be called by the user of the library, that is
why it uses the Nest Function Signature.

Now let's take a look at `my_lib.cpp`:

```better-c
#include "my_lib.h"

#define FUNC_COUNT 1 // define here the number of functions inside the library

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;
    Nst_OpErr err = { nullptr, nullptr };

    func_list_[idx++] = NST_MAKE_FUNCDECLR(add, 2);

#if __LINE__ - FUNC_COUNT != 15 // keep this number as the line after the first
                                // func_list_[idx++] = ...
#error
#endif

    lib_init_ = err.name == nullptr;
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

NST_FUNC_SIGN(add)
{
    Nst_Int a, b;
    NST_DEF_EXTRACT("ii", &a, &b);
    return nst_int_new(a + b, err);
}
```

Before addressing anything else, look at the implementation of `get_func_ptrs`;
it will probably never be modified, as a matter of fact all the standard library
uses that format.

Now let's address the elephant in the room: `#if __LINE__ - FUNC_COUNT != 15`.
This is there to prevent mistakes before the compilation takes place, having
`FUNC_COUNT` set to the wrong amount will lead to crashes and is better to
update a number
