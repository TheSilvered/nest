# Creating your own C library

In this tutorial we will create a small 2D vector library, to show some of the
functions available.

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
├── build # various files to compile Nest and the installers
├── docs # the documentation
├── examples # some examples of Nest code
├── include # the header files for the core interpreter
├── libs # the standard libraries
├── plugins # the plugins for SublimeText and VSCode
├── src # the source of the core interpreter
├── tests # the tests for the libraries and the interpreter
└── tools # tools for developing Nest
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
library called `vec_lib` and put it in `~/vec_lib/`.

```text
$ nest mkclib.nest new vec_lib ~/
Copied headers
Updated Windows x86 Debug
Updated Windows x86 Release
Updated Windows x64 Debug
Updated Windows x64 Release
Updated Linux x64/x86 Debug
Updated Linux x86 Release
Updated Linux x64 Release
Created 'vec_lib.sln'
Created 'makefile'
Created 'vec_lib.nest'
Created 'vec_lib.vcxproj'
Created 'vec_lib.vcxproj.filters'
Created 'dllmain.cpp'
Created 'framework.h'
Created 'vec_lib.h'
Created 'vec_lib.cpp'
'vec_lib' created succesfully!
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

Now that the library has been created you can navigate to `~/vec_lib/vec_lib`
to access the source files. `dllmain.cpp` and `framework.h` are used on Windows
to generate the DLL and `vec_lib.vcxproj` and `vec_lib.vcxproj.filters` are
project files used by Visual Studio.  
Furthermore, if you navigate to `~/vec_lib/nest_source` you will find all the
necessary headers.

## Analyzing the generated files

The other two files inside `~/vec_lib/vec_lib` are `vec_lib.h` and `vec_lib.cpp`
that are the ones that you will want to modify.

`vec_lib.h` contains the following:

```better-c
#ifndef VEC_LIB_H
#define VEC_LIB_H

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

#endif // !VEC_LIB_H
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

Now let's take a look at `vec_lib.cpp`:

```better-c
#include "vec_lib.h"

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

First, let's look at the implementation of `lib_init`. The first two lines are
there to make it easier to set the values of `func_list_` and to catch any
errors, then line 14 declares the function `add` as a variable to be exported
as a function that accepts two arguments.  
Now let's address the elephant in the room: `#if __LINE__ - FUNC_COUNT != 15`.
This is quite odd but is there to prevent `FUNC_COUNT` from being incorrect and
causing a write-out-of-bounds operation because the `func_list_` array is too
small. The program will not compile, though, when you add any new statements
above this line but I reckon is more desirable to fail to compile than to crash
at run-time.

Moving our attention at the implementation of `get_func_ptrs`; it will probably
never be modified, as a matter of fact all the standard library uses that format.

This said, let's take a look at the implementation of `add`. Adding two numbers
is not a hard task: firstly the values passed from Nest are extracted, as they
are in the `args` array, and then a new `Int` object is created from the sum
of the values of the two arguments. I suggest reading the documentation of
[`NST_DEF_EXTRACT`](../c_api/c_api-lib_import.md#nst_def_extract) and of the
other functions and macros declared in `lib_import.h` as they were originally
thought for external libraries.

## Creating the `Vec2` type

The first step to creating this library is to make a new `Type` object that will
be assigned to the instances of the new `Vec2` objects. To create it let's first
make a global static variable called `t_Vec2` of type `Nst_TypeObj *` to contain
the new `Type` object. Then we can replace the declaration of `add` with this
object declaration:

```better-c
func_list_[idx++] = NST_MAKE_NAMED_OBJDECLR(t_Vec2, "Vec2");
```

This creates a new variable when the module is imported with the name `Vec2` and
the value of `t_Vec2`. But `t_Vec2` has not been yet initialized, that is why
we add this line after before the declaration and change line 18:

```better-c
t_Vec2 = nst_type_new("Vec2", 4, &err);
```

The last thing to do is implement `free_lib` since we initialized a new object
in `lib_init`.

```better-c
void free_lib()
{
    nst_dec_ref(t_Vec2);
}
```

Without forgetting to export it in `vec_lib.h`.

To test that everything is working, compile it from Visual Studio or with `make`
and open `vec_lib.nest` and replace the contents with

```nest
|#| '__C__:vec_lib.cnest' = vec_lib

>>> (vec_lib.Vec2 '\n' ><)
```

changing `vec_lib.cnest` depending on what version of Nest you have installed,
since I have the 64-bit release on Windows I will put `x64/Release/vec_lib.dll`.

Now, if you run it you should see `Vec2` being printed on the console.

!!!note
    When importing a C library directly you need to add `__C__` in front of the
    path otherwise Nest will attempt to load it as a `.nest` file.

## Creating the `Vec2` object

Now we can move on to create the actual `Vec2` object. The first thing to do is
creating the struct that will represent it, so in `vec_lib.h` add this before
the function declarations:

```better-c
typedef struct _Vec2Obj
{
    NST_OBJ_HEAD;
    f64 x, y;
}
Vec2Obj;
```

Here, `NST_OBJ_HEAD` adds all the fields to the struct to be able to manage it
as an object and `x` and `y` are just additional fields.

Then let's declare a new function to create an instance of this object:

```better-c
Nst_Obj *vec2_new(f64 x, f64 y, Nst_OpErr *err);
```

and implement it in `vec_lib.cpp`

```better-c
Nst_Obj *vec2_new(f64 x, f64 y, Nst_OpErr *err)
{
    Vec2Obj *vec2 = nst_obj_alloc(Vec2Obj, t_Vec2, nullptr, err);
    if ( vec2 == nullptr )
    {
        return nullptr;
    }

    vec2->x = x;
    vec2->y = y;

    return OBJ(vec2);
}
```

As you can see the last argument is a `Nst_OpErr *` because the memory
allocation might fail.

Now that we have the `Vec2` object ready, we can start working onto implementing
the functions that the user will be able to call from Nest.

## Implementing `vec2` and `vec2_to_str`

Now that we can create `Vec2` objects in C, we should let the user do it too! To
do this we need a wrapper function for `vec2_new` that is callable by Nest
simply called `vec2` by adding `NST_FUNC_SIGN(vec2);` to `vec_lib.h` and
the implementation to `vec_lib.cpp`:

```better-c
NST_FUNC_SIGN(vec2)
{
    Nst_Real x, y;
    NST_DEF_EXTRACT("NN", &x, &y);
    return vec2_new(x, y, err);
}
```

Now let's make it available to the user by adding this line to `lib_init`:

```better-c
func_list_[idx++] = NST_MAKE_FUNCDECLR(vec2, 2);
```

and updating `FUNC_COUNT`. Note that the second argument, `2` is the number of
arguments that the function takes.

Now we can try to test it and you should be able to create the vectors but if
you try to print them you will probably be greeted with something like
`<Vec2 object at 0x000001B2A47C36F0>`. That is not ideal, and since there is no
way to see its contents. So let's create another function just for that called
`vec2_to_str`. Here I will just show you the implementation and you can add
everything else just like you did for `vec2`.

```better-c
NST_FUNC_SIGN(vec2_to_str)
{
    Vec2Obj *vec2;
    NST_DEF_EXTRACT("#", t_Vec2, &vec2);
    return nst_sprintf("<Vec2 x=%lg y=%lg>", vec2->x, vec2->y);
}
```

## Implementing other functions

Now that the basic functions are in place, you can start implementing various
functions to get back the `x` and `y` values, to normalize, to get the length or
the cross product etc... all following the general process showed above.

To see my implementation of the library you can check it inside the examples
directory in the repository.

## Nest wrapper

After you have implemented all the functions you want to include, you should
create a wrapper to the library to let the user import a `.nest` file instead of
the `.cnest` one.

!!!note
    In Windows, even though the library is compiled as a `.dll` you should
    change the extension to `.cnest` as that is the standard for Nest C
    libraries.
