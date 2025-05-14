# A quick tour

## Hello, World

This is one of the first programs everyone writes when learning a new
programming language. In Nest Hello, World is written like this:

```nest
>>> 'Hello, World!\n'
```

Here `>>>` is an operator that prints to the standard output (there is also
`<<<` that will read from the standard input!) and
`'Hello, World!\n` is a string with a newline character at the end.

To avoid writing the newline the standard library has a function that will do
just that.

```nest
|#| 'std.nest' = std

'Hello, World!' @std.io.println
```

You can ignore the syntax that is used for importing the standard library and
calling the function for now, just know that this program will print any
expression before `@std.io.println`.

## Numbers and arithmetic operations

Operators in Nest are similar to the ones in lisp even if the syntax is a little
different. This program, for example adds the numbers `1`, `2`, and `3` and
prints them:

```nest
|#| 'std.nest' = std

1 2 3 + @std.io.println
```

An operator that takes all the values that precede it is called a 'stack
operator' because you can think of the values as being pushed on a stack and
the operator clearing them and pushing the result back. Other stack operators
include

- `*` for multiplication,
- `-` for subtraction,
- `/` for division,
- `^` for exponentiation and
- `==` for equality.

!!!note
    Check the full list of stack operators in
    [general syntax](syntax.md/#stack-operators)

You can limit the amount of arguments an operator consumes using parenthesis:

```nest
|#| 'std.nest' = std

(2 2 ^) (2 2 *) == @std.io.println
```

This program will output `true`.

This expression is normally written as `2^2 == 2*2` (or `2**2 + 2*2`) and
strictly speaking the first pair of parenthesis is redundant but it makes the
code more readable showing clearly the order of operations.

## Local operators

In Nest there is a second kind of operator called 'local operator'. It operates
only on the argument directly to its right, for example the boolean not `!`
will make this program print `false`

```nest
|#| 'std.nest' = std

!true @std.io.println
```

We have already seen two other local operators in this tour, one is `>>>` that
prints its argument to the standard output and the other one is `|#|` which
imports a library.

!!!note
    Check the full list of local operators in
    [general syntax](syntax.md/#local-operators)

## Comments

In Nest comments begin with a double dash (`--`) and last until the end of the
line, there are also multiline comments that are opened with `-/` and closed
with `/-`

```nest
|#| 'std.nest' = std

'The result of 3 + 5 is:' @std.io.println
-- 10 @std.io.println
8 @std.io.println
```

## Variables

Variables in Nest can be assigned with an equal `=` sign but, differently from
other languages, the value is written first and the name last.

```nest
|#| 'std.nest' = std

2 = a

a @std.io.println --> prints 2
```

You can also change the variable using compound assignment operators:

```nest
|#| 'std.nest' = std

2 = a
a @std.io.println --> prints 2
3 -= a
a @std.io.println --> prints -1
```

Assignment is an expression so you can chain various assignments together:

```nest
|#| 'std.nest' = std

2 = a = b
a @std.io.println --> prints 2
b @std.io.println --> prints 2

1 b += a -- compound assignments behave like stack operators
a @std.io.println --> prints 5
```

Accessing a variable that does not exist results in a special `null` object:

```nest
|#| 'std.nest' = std

not_defined @std.io.println --> prints null
```

## Conditional statements

Conditional statements are written like ternary expressions in C or JavaScript:

```nest
|#| 'std.nest' = std

(2 2 == ? 'Math works' : 'Check your code') @std.io.println
```

This program will hopefully write `Math works` to the console.

The parenthesis in this program are needed as conditional expressions are
firstly statements which also happen to evaluate to a value. If the parenthesis
were not there the code would have been the equivalent of this:

```nest
|#| 'std.nest' = std

2 2 == ? 'Math works' : ('Check your code' @std.io.println)
```

But writing single expressions is clunky and hard to read, to execute a
multiline block surround multiple statements with square brackets:

```nest
|#| 'std.nest' = std

2 2 == ? [
    'Math works' @std.io.println
    'And this line prints too' @std.io.println
] : [
    'Check your code' @std.io.println
    'How does 2 not equal 2' @std.io.println
]
```

## Conditional loops

Conditional loops in Nest can have two forms: `?..` and `..?` respectively. The
first checks the condition before executing what is inside the body (a `while`
loop) and the second executes its body before checking the condition (a
`do-while` loop).

```nest
|#| 'std.nest' = std

-- print the numbers from 1 to 5

1 = i
?.. i 5 > [
    i @std.io.println
    1 += i
]
```

## Collections

In Nest there are multiple types of collections.

### Strings

Strings are a sequence of Unicode characters. They are ordered and immutable. To
write a string you can use both single quotes (`'`) and double quotes (`"`).

```nest
'This is a string'
"This is also a string"
```

Strings support various common escape sequences such as `\n` for newlines and
`\t` for tabs. Nest also allows the interpolation of an expression with
`\(...)` (the expression is put in place of the `...`).

```nest
|#| 'std.nest' = std

'Nest' = var
'Hello, \(var)!' @std.io.println --> prints Hello, Nest!
```

To join multiple strings together use the `><` operator, this works with
objects of any type and converts them to a string first:

```nest
|#| 'std.nest' = std

3 = apples

'I have ' apples ' apples' >< @std.io.println --> prints I have 3 apples
```

### Vectors and arrays

These are ordered sequences that can contain objects of any type. The
difference between vectors and arrays is that the first has a variable length
and the second one does not.

```nest
{1, 2, 'an array'}
<{3, 'a vector'}>
```

!!!note
    Empty arrays are written as `{,}` because `{}` are empty maps.

### Hash maps

Hash maps contain key-value pairs with unique keys. They are ordered.

```nest
{'first': 1, 2: 'second'}
```

## Working with collections

### Accessing items

To access an item inside a collection you can use a dot (`.`). The object after
the dot is then used to access the collection. For strings, vectors and array
an integer is expected to be used as an index, maps expect a key instead.

If an identifier is fount right after the dot it is interpreted as a string, so
`my_map.key` is the same as `my_map.'key'`. To interpret the identifier as a
variable name use parenthesis: `my_map.(key)`.

```nest
|#| 'std.nest' = std

{'a', 2, 'c'} = my_arr
<{1, 'b', 3}> = my_vec
{'key': 'value', 1: 2} = my_map
'string' = my_str

1 = var

my_arr.0 @std.io.println --> prints a
my_vec.2 @std.io.println --> prints 3
my_map.key @std.io.println --> prints value
my_map.(var) @std.io.println --> prints 2
my_str.4 @std.io.println --> prints n
```

Negative indices work too where `-1` is the last item of a sequence, `-2` the
second to last and so on:

```nest
|#| 'std.nest' = std

{'first', 'second', 'last'} = arr
'❗❌✅' = str

arr.-1 @std.io.println --> prints last
str.-1 @std.io.println --> prints ✅
```

Using this notation you can also modify the items inside a collection, with the
exception of strings as they are immutable:

```nest
|#| 'std.nest' = std

{'a', 2, 'c'} = vec
vec @std.io.println --> prints {'a', 2, 'c'}
'b' = vec.1
vec @std.io.println --> prints {'a', 'b', 'c'}

{'name': 'Nets', 'type': 'language'} = info
info @std.io.println --> prints {'name': 'Nets', 'type': 'language'}
'Nest' = info.name
info @std.io.println --> prints {'name': 'Nest', 'type': 'language'}
```

### Adding items

To append one or more values to a vector you can use the `+` operator:

```nest
|#| 'std.nest' = std

<{}> = vec
vec @std.io.println --> prints <{}>
vec 2 3 +
vec @std.io.println --> prints <{2, 3}>
```

To add a new key-value pair to a map just assign the new key to the value:

```nest
|#| 'std.nest' = std

{1: 'st', 3: 'rd'} = map
map @std.io.println --> prints {1: 'st', 3: 'rd'}
'nd' = map.2
map @std.io.println --> prints {1: 'st', 3: 'rd', 2: 'nd'}
```

### Removing items

To remove a pair from a map use the `-` operator with the map followed by the
key or keys you want to remove, this operation results in the map itself.

```nest
|#| 'std.nest' = std

{1: 'st', 2: 'nd', 3: 'rd'} = map
map @std.io.println --> prints {1: 'st', 2: 'nd', 3: 'rd'}
map 2 3 - = result
map @std.io.println --> prints {1: 'st'}
result @std.io.println --> prints {1: 'st'}
```

To remove an item from a vector you can also use the `-` and this will remove
the first matching item, this operation results in the vector itself.

```nest
|#| 'std.nest' = std

<{1, 2, 3}> = vec
vec @std.io.println --> prints <{1, 2, 3}>
vec 1 3 - = result
vec @std.io.println --> prints <{2}>
result @std.io.println --> prints <{2}>
```

To remove items from the end of a vector use `/` to pop them off. This
operation results in the last item popped.

```nest
|#| 'std.nest' = std

<{1, 2, 3, 4}> = vec
vec @std.io.println --> prints <{1, 2, 3, 4}>

vec 1 / = result
vec @std.io.println --> prints <{1, 2, 3}>
result @std.io.println --> prints 4

vec 2 / = result
vec @std.io.println --> prints <{1}>
result @std.io.println --> prints 2
```

!!!note
    More complex operations can be performed with the functions in the
    [`stdsequtil.nest`](stdlib/sequence_utilities_library.md) standard library,
    accessible as `std.sequ` when importing `|#| 'std.nest' = std`.

## Iterative loops

In addition to conditional loops, Nest has iterative loops written with `...`.
Using them you can iterate through a collection:

```nest
|#| 'std.nest' = std

'My string' = s
... s := char [
    char @std.io.println
]

{
    'My': 'map',
    'first': 1,
    2: 'second'
} = m

... s := pair [
    pair @std.io.println
]
```

You can also iterate through a range using the `start -> end` notation or
`start step -> end` to add a step to each iteration.

```nest
|#| 'std.nest' = std

... 0 2 -> 11 := num [
    num @std.io.println
]
```

## Functions

### Definition

To define a function in Nest you can use a hash sign (`#`):

```nest
|#| 'std.nest' = std

#greet [
    'Hello, world!' @std.io.println
]
```

In this example the function `greet` prints `Hello, world!` and to actually call
it you can use an at sign (`@`):

```nest
|#| 'std.nest' = std

#greet [
    'Hello, world!' @std.io.println
]

@greet --> prints Hello, world!
```

### Arguments

A function can take any number of arguments which will be able to access when it
is called. The arguments are passed to the function before the `@`:

```nest
|#| 'std.nest' = std

#greet name [
    'Hello, \(name)!' @std.io.println
]

'Nest' @greet --> prints Hello, Nest!
```

If you do not pass enough arguments to a function the missing ones will be set
to `null`:

```nest
|#| 'std.nest' = std

#greet name [
    'Hello, \(name)!' @std.io.println
]

@greet --> prints Hello, null!
```

### Returning a value

A return statement is introduced by `=>` and will return the value that follows
it. If a function has no explicit return it will implicitly return `null`.

```nest
|#| 'std.nest' = std

#greet name [
    => 'Hello, \(name)!'
]

'Nest' @greet @std.io.println --> prints Hello, null!
```

If the only statement of the function is the return then you can avoid the
square brackets:

```nest
|#| 'std.nest' = std

#greet name => 'Hello, \(name)!'

'Nest' @greet @std.io.println --> prints Hello, null!
```

If after the `=>` you do not write anything the function will return `null`:

```nest
|#| 'std.nest' = std

#greet name [
    name 'Nest' == ?
        =>
    :
        => 'Hello, \(name)!'
]

'Nest' @greet @std.io.println --> prints null
```

### Anonymous functions

To define an anonymous function you use two hashes instead of one:

```nest
|#| 'std.nest' = std

(## name => 'Hello, \(name)!') = greet

'Nest' @greet @std.io.println --> prints null
```

### Outer scopes

A function will always reference the variables of the module where it is
defined. For example:

```nest
|#| 'std.nest' = std

#greet name [
    'Hello, \(name)!' @std.io.println
]

'Nest' @greet --> prints Hello, Nest!
```

Here the function `greet` is referencing `std` from the global scope.

If a function is defined inside another function it will be able to reference
any variable defined before its definition:

```nest
|#| 'std.nest' = std

#outer [
    1 = outer_var

    #inner [
        outer_var @std.io.println --> prints 1
        outer_var_2 @std.io.println --> prints null
    ]

    2 = outer_var_2
    => inner
]

@(@outer) --> prints `1` on the first line and `null` on the second
```

### Classes

Nest does not have classes as a special language construct, rather they are a
result of a clever trick you can do with functions. Take the following example:

```nest
#vec2 x y [
    {
        'x': x,
        'y': y
    } = self

    ## [
        => '(\(self.x), \(self.y))'
    ] = self.to_str

    ## other [
        => (self.x other.x +) (self.y other.y) @vec2
    ] = self.add

    ## factor [
        factor *= self.x
        factor *= self.y
    ] = self.scale

    => self
]
```

This can be considered a very rudimentary class. All functions can reference
`self` and since it is the same object they can change its contents and see the
changes made by another function.

```nest
|#| 'std.nest' = std

#vec2 x y [...]

2.0 4.0 @vec2 = vec
@vec.to_str @std.io.println --> prints (2.0, 4.0)
0.5 @vec.scale
@vec.to_str @std.io.println --> prints (1.0, 2.0)

4.0 3.0 @vec2 @vec.add = sum
@sum.to_str @std.io.println --> prints (5.0, 5.0)
```
