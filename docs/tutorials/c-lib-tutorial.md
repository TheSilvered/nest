# Creating your own C library

In this tutorial we will create a small 2D vector library, to show some of the
functions available.

## Installation of the source

First things first you need to clone the latest stable release of Nest by
executing:

```text
git clone -b stable https://github.com/TheSilvered/nest.git
cd nest
```

Then you can compile it from source as specified in the
[installation](../installation.md) documentation.

Here is a little explanation of what all the folders contain:

```text
nest
├── build -- various files to compile Nest and the installers
├── docs -- the documentation
├── examples -- some examples of Nest code
├── include -- the header files for the core interpreter
├── libs -- the standard libraries
├── plugins -- the plugins for SublimeText and VSCode
├── src -- the source of the core interpreter
├── tests -- the tests for the libraries and the interpreter
└── tools -- tools for developing Nest
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

NstEXP bool NstC lib_init();
// NstEXP void NstC lib_quit();

Nst_Obj *NstC add(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VEC_LIB_H
```

Here, excluding boilerplate code, you will see three function declarations,
with the second one being commented out. The first two functions (`lib_init`,
and `lib_quit`) are used by the interpreter to communcate with the library as
follows:

- `lib_init` is the function that is called when the library is first loaded
  and should initialize any objects that last for the lifetime of the library,
  it returns the list of objects exported by the library and `NULL` if the
  library fails to be initialized
- `lib_quit` is optional and is called when the main library is unloaded, this
  function should de-initialize anything initialized with `lib_init`, this
  function must not throw any error

Now we can get to the last function: this kind of function is what can be
called by the user of the library. Note that it takes two arguments: the number
of arguments being passed to the function (usually called `arg_num`) and the
array of the arguments themselves (usually called `args`). The arguments are
always passed exclusively as Nest objects.

Now let's take a look at `vec_lib.cpp`:

```better-c
#include "vec_lib.h"

static Nst_Declr obj_list_[] = {
    NST_FUNCDECLR(add, 2),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

Nst_Obj *NstC add(usize arg_num, Nst_Obj **args)
{
    i64 a, b;
    if (!Nst_extract_args("ii", arg_num, args, &a, &b))
        return nullptr;
    return Nst_int_new(a + b);
}
```

First let's look at the function list: here it is called `obj_list_` since it
can also contain object declarations. This is an array of `Nst_Declr` structs.
Inside you can see the `add` function declared in `vec_lib.h` added to the
array through `Nst_FUNCDECLR`, this macro takes the function as the first
argument and the number of arguments it takes as the second, in this case the
`add` function takes two arguments. When using this macro the function is
accessible through Nest with the same name as in C, if you instead want to
use a different name check
[`Nst_NAMED_FUNCDECLR`](../c_api/c_api-lib_import.md#nst_named_funcdeclr).
At the end of the array there must always be a `Nst_DECLR_END` to signal to
Nest that there are no more functions to export.

Now, let's look at the implementation of `lib_init`. Since this library
doesn't yet need to initialize anything it returns the list of functions
(called `obj_list_`) directly.

Lastly, let's take a look at the implementation of `add`. Since the values are
passed as Nest objects they need to be extracted to be used in C, for that
Nest provides the
[`Nst_extract_args`](../c_api/c_api-lib_import.md#nst_extract_args) function.
Check the documentation do see how it works, in this case it extracts two
integers into `a` and `b`. Then, to return the result we create a new `Int`
object with the sum as its value and we return it.

## Creating the `Vec2` type

The first step to creating this library is to make a new `Type` object that
will be assigned to the instances of the new `Vec2` objects. To create it let's
first make a global static variable called `t_Vec2` of type `Nst_TypeObj *` to
contain the new `Type` object. Now in `lib_init` we can initialize it like so:

```better-c
t_Vec2 = Nst_type_new("Vec2");
```

And since `Nst_type_new` can fail we should check for any errors and return
`nullptr`.

```better-c
if (t_Vec2 == nullptr)
    return nullptr;
```

Now we need to add a `lib_quit` function that will remove the reference of the
object from `t_Vec2`:

```better-c
void lib_quit()
{
    Nst_dec_ref(t_Vec2);
}
```

Without forgetting to export it in `vec_lib.h`.

Now that we have the variable we need to export it to Nest. To do this we first
need to create a function that returns an instance of the type, this function
does not take any arguments:

```better-c
Nst_Obj *Vec2()
{
    return Nst_inc_ref(t_Vec2);
}
```

Going back to `obj_list_` we can remove the declaration for `add` and add one
for `Vec2` with `Nst_CONSTDECLR(Vec2)`, note that `Vec2` is the name of the
function and is the name with which the type will be available through Nest.
If you have called the function with a different name you can use
`Nst_NAMED_CONSTDECLR(other_func, "Vec2")` to export the same name.

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
    When importing a C library directly you need to add `__C__:` in front of
    the path otherwise Nest will attempt to load it as a `.nest` file.

## Creating the `Vec2` object

Now we can move on to create the actual `Vec2` object. The first thing to do is
creating the struct that will represent it, so in `vec_lib.h` add this:

```better-c
typedef struct _Vec2Obj {
    Nst_OBJ_HEAD;
    f64 x, y;
} Vec2Obj;
```

Here, `Nst_OBJ_HEAD` adds all the fields to the struct to be able to manage it
as a Nest object and `x` and `y` are just additional fields.

Then let's declare a new function to create an instance of this object:

```better-c
Nst_Obj *vec2_new(f64 x, f64 y);
```

and implement it in `vec_lib.cpp`

```better-c
Nst_Obj *vec2_new(f64 x, f64 y)
{
    Vec2Obj *vec2 = Nst_obj_alloc(Vec2Obj, t_Vec2, nullptr);
    if (vec2 == nullptr)
        return nullptr;

    vec2->x = x;
    vec2->y = y;

    return OBJ(vec2);
}
```

Now that we have the `Vec2` object ready, we can start working onto
implementing the functions that the user will be able to call from Nest.

## Implementing `vec2` and `vec2_to_str`

Now that we can create `Vec2` objects in C, we should let the user do it too!
To do this we need a wrapper function for `vec2_new` that is callable by Nest
simply called `vec2`:

```better-c
Nst_Obj *NstC vec2(usize arg_num, Nst_Obj **args)
{
    f64 x, y;
    Nst_DEF_EXTRACT("NN", &x, &y);
    return vec2_new(x, y);
}
```

Now let's make it available to the user by adding `Nst_FUNCDECLR(vec2, 2)` to
`obj_list_`.

Now we can try to test it and you should be able to create the vectors:

```nest
|#| '__C__:vec_lib.cnest' = vec_lib

2 3 @vec_lib.vec2 = vec
>>> (vec '\n' ><)
```

But if you try to print them you will probably be greeted with something like
`<Vec2 object at 0x000001B2A47C36F0>`. That is not ideal, and since there is no
way to see its contents let's create another function just for that called
`vec2_to_str`. Here I will just show you the implementation and you can add
everything else just like you did for `vec2`.

```better-c
Nst_Obj *NstC vec2_to_str(usize arg_num, Nst_Obj **args)
{
    Vec2Obj *vec2;
    Nst_DEF_EXTRACT("#", t_Vec2, &vec2);
    return Nst_sprintf("<Vec2 x=%lg y=%lg>", vec2->x, vec2->y);
}
```

## Implementing other functions

Now that the basic functions are in place, you can start implementing various
functions to get back the `x` and `y` values, to normalize, to get the length
or the cross product etc... all following the general process showed above.

To see my implementation of the library you can check it in the
[repository](https://github.com/TheSilvered/nest/tree/main/examples/vec_lib).

## Nest wrapper

After you have implemented all the functions you want to include, you should
create a wrapper to the library to let the user import a `.nest` file instead
of the `.cnest` one.

!!!note
    In Windows, even though the library is compiled as a `.dll` you should
    change the extension to `.cnest` to allow the `.nest` wrapper to be
    cross-platform.
