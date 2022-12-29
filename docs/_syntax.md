# General nest syntax

## Comments

In Nest there are two kinds of comments: line comments and block comments.
Line comments start with `--` and end at the end of the line. If there is a
backslash at the end, the line feed is escaped and the line comment continues
to the next line.

Block comments start with `-/` and end with `/-`

```text
-- This is a single line comment
-- This is another single line comment \
   that has the line-feed escaped

-/ this is a block comment which
   spans multiple lines without
   any escapes /-
```

## Value literals

### Numeric literals

Integer literals can be decimal, binary, octal or hexadecimal. The first one has
no prefix, the second `0b`, the third `0o` and the fourth `0x`.

Binary and octal literals cannot be directly followed by another integer and
hexadecimal literals cannot be followed by a word.

Real number literals support scientific notation but must always have some
digits both before and after the dot.

Both integer and real literals can be prefixed with a minus `-` to make them
negative. There can also be a plus sign `+` before the number but that does not
change its sign.

```text
-- Integer literals
10
-123
012 -- equal to 12 but still valid
+11
0b101
0o377
0xab

0b102 -- invalid because followed by 2
0o159 -- invalid because followed by 9
0xabg -- invalid because followed by g

-- Real literals
0.2
-13.4
1.0
+38.1
1.2e10
1.2e-10
1. -- invalid
.3 -- also invalid
3e10 -- invalid because real literals need always a dot
```

Nest has also a byte type that can be written as an integer literal followed by
a lowercase `b` or an uppercas one `B`. If the integer is written with an
hexadecimal literal, since adding a `b` at the end would be counted as digit,
you start with `0h` instead.

```text
-- Byte literals
10b
256b -- equivalent to 0b
0b101b
0o123b
0hff
```

Decimal, binary and octal byte literals can be followed by a word or by another
literal but that could cause confusion, so it is better to keep a space.

```text
10bab -- split into '10b' and 'ab'
0b0b0 -- split into '0b' (from '0b0b') and '0'
00b10 -- split into '0b' (from '00b') and '10'

-- these are much clearer
10b ab
0b0b 0
00b 10
```

### String literals

String literals begin and end with either a single or a double quote but there
is a difference between the two: strings with double quotes can span multiple
lines.

To escape a character you can use a backslash `\` before the character and here
are all the valid escape sequences:

| Sequence | Hex Value | Name                   |
|----------|-----------|------------------------|
| `\\`     | `5c`      | Backslash              |
| `\'`     | `27`      | Single quote           |
| `\"`     | `22`      | Double quotes          |
| `\a`     | `07`      | Alert / Bell           |
| `\b`     | `08`      | Backspace              |
| `\e`     | `1b`      | Backspace              |
| `\f`     | `0c`      | Form feed / Page break |
| `\n`     | `0a`      | Line feed / Newline    |
| `\r`     | `0d`      | Carriage return        |
| `\t`     | `09`      | Horizontal tab         |
| `\v`     | `0b`      | Vertical tab           |
| `\xhh`   | None      | Hexadecimal byte, `hh` represents a hex number from `00` to `ff` |
| `\nnn`   | None      | Octal byte, `nnn` represents an octal number from `0` to `777` |

> NOTE: octal escapes can have either one, two or three digits, `\12` and `\012`
> are both valid escapes

### Array literals

Array literals start and end with braces (`{` and `}`) and inside have various
expressions separated by a comma.

```text
{ 1, 2, 3, 4 }
```

> NOTE: writing `{ }` creates an empty map, not an empty array. To create an
> empty array you can write `Array :: <{ }>`

You can also create an array with all of the same value like this:

```text
{ 10;30 } --> array of length 30, with all values that are 10
```

This puts the same object in all slots of the array meaning that if an
object can change values inside of itself (array, vector or map) the values of
all objects will change:

```text
{ { 1, 2 };2 } = a --> 'a' is equal to { { 1, 2 }, { 1, 2 } }
3 = a.0 .0 --> 'a' is now equal to { { 3, 2 }, { 3, 2 } }
```

### Vector literals

Vector literals start with `<{` and end with `}>` and inside have various
expressions separated by a comma.

```text
<{ 1, 2, 3, 4 }>
```

Just like arrays, vectors also have a smaller syntax to fill all slots with the
same value:

```text
<{ 10;30 }> --> vector of length 30, with all values that are 10
```

### Map literals

Map literals start with `{` and end with `}` and inside have key-value pairs.

```text
{ 'key_1': 1, 'key_2': 2 }
```

### Anonymous functions (lambdas)

Lambdas are one-expression functions that have no name. Their syntax is the
following:

```text
##arg1 arg2 arg3 => expression
```

If you want to store a lambda in a variable you need parenthesis otherwise the
assignment will be included in the expression returned.

```text
##a => a 1 + = func -- wrong, func is assigned inside the return expression

(##a => a 1 +) = func -- correct, func now holds the lambda
```

## Predefined variables

- `Int`: integer type
- `Real`: real number type
- `Bool`: boolean type
- `Null`: null type, absence of a value
- `Str`: ASCII string type
- `Array`: array type, non-resizable ordered collection of objects
- `Vector`: vector type, resizable ordered collection of objects
- `Map`: hash map type, holds key-value pairs
- `Func`: function type
- `Iter`: iterator type
- `Byte`: byte type, an integer from 0 to 255
- `IOfile`: file type, similar to `FILE *` in C
- `Type`: type of all types
- `true`: boolean true
- `false`: boolean false
- `null`: the only possible value of type `Null`
- `_cwd_`: a string showing the current working directory
- `_args_`: an array with the arguments passed in the command line
- `_vars_`: a table containing the variables of the local scope
- `_globals_`: a table containing the variables of the global scope

> NOTE: the value of `_globals_` in the global scope is `null`

## Expressions

### Stack operators

An expression in Nest works in a similar way as it does in Lisp, where there are
some operators which can operate with an indeterminate number of operands; these
are called stack operators.

The stack operators in Nest are:

- `+` additiontext
- `-` subtraction
- `*` multiplication
- `/` division
- `^` exponentiation
- `%` reminder
- `><` concatenation
- `&&` logical and
- `||` logical or
- `&|` logical xor
- `&` bit-wise and
- `|` bit-wise or
- `^^` bit-wise xor
- `<<` bit-wise left shift
- `>>` bit-wise right shift
- `>` greater than
- `>=` greater than or equal to
- `<` less than
- `<=` less than or equal to
- `==` equality
- `!=` inequality

To perform an operation with these operators you can write all the operands
followed by the operator:

```text
1 2 3 +
2 8 ^
```

To prioritize certain operations or to limit the number of operands of an
operator, you can use parenthesis:

```text
(3 2 ^) (4 2 ^) + 0.5 ^
```

this expression is equal to the python expression `(3 ** 2 + 4 ** 2) ** 0.5`.

The comparison operators (`<`, `<=`, `>`, `>=`, `==` and `!=`) operate in a
different way though. They check each pair of adjacent operands and then check
all the results to see if they are all true.

The following two expressions are equal but the first one is more concise:

```text
1 2 3 4 <
(1 2 <) (2 3 <) (3 4 <) &&
```

> NOTE: writing `a b c !=` does not mean that all three values are different
> from each other, it only means that adjacent ones are.

The operators `+`, `-`, `*` and `/` have a different meaning when operating in
vectors.

The `+` operator appends any value to the end of a vector and returns the vector
itself.

The `-` operator removes the first occurrence of a value in a vector and returns
the vector itself.

The `*` operator repeats the contents of a vector a number of times and returns
the vector itself.

The `/` operator pops a number of values from the end and returns the last value
popped.

```text
>>> (<{ 1, 2, 3 }> 2 + '\n' ><) --> <{ 1, 2, 3, 2 }>
>>> (<{ 1, 2, 3 }> 2 - '\n' ><) --> <{ 1, 3 }>
>>> (<{ 1, 2, 3 }> 2 * '\n' ><) --> <{ 1, 2, 3, 1, 2, 3 }>
>>> (<{ 1, 2, 3 }> 2 / '\n' ><) --> 2
```

The operator `-` behaves differently with maps too. It will remove a key from a
map and return the map itself.

```text
{ 'a': 1, 'b': 2 } = m
>>> (m 'a' - '\n' ><) --> { 'b': 2 }
>>> (m 'j' - '\n' ><) --> { 'b': 2 } removing a key that does not exist does not
                      --             throw an error
```

### Local operators

In Nest there is another type of operator that is called local operator. A local
operator only operates with the operand on the right.

The local operators in Nest are:

- `$` length of an object
- `!` logical not
- `~` bit-wise not
- `>>>` write to the standard output
- `<<<` write to the standard output and read from the standard input
- `-:` negate
- `?::` type of
- `@@` local call, calls a function with no arguments
- `|#|` import

To operate with these operators you can write the operator followed by its
operand:

```text
>>> 'Hello, world!\n'
```

### Local-stack operators

This third type of operator in Nest can take a set number of arguments,
depending on the operator you are using. These operators are used do to specific
functions built in the language.

The local-stack operators are:

- `::` casts an object to another type
- `@` calls a function
- `*@` calls a function with an array or a vector containing the arguments
- `->` creates an integer iterator
- `!!` throws an error

To use a local-stack operator you write the last argument to the right and all
the others to the left.

`::` only takes two arguments: the object to cast and the type to cast it to.

Here the number `10` is casted to a `Byte` object:

```text
Byte :: 10
```

The valid type casts in nest are the following:

| ↱        | `Int` | `Real` | `Bool` | `Null` | `Str` | `Array` | `Vector` | `Map` | `Func` | `Iter` | `Byte` | `IOFile` | `Type` |
|----------|-------|--------|--------|--------|-------|---------|----------|-------|--------|--------|--------|----------|--------|
| `Int`    |   X   |   X    |   X    |        |   X   |         |          |       |        |        |   X    |          |        |
| `Real`   |   X   |   X    |   X    |        |   X   |         |          |       |        |        |        |          |        |
| `Bool`   |       |        |   X    |        |   X   |         |          |       |        |        |        |          |        |
| `Null`   |       |        |   X    |   X    |   X   |         |          |       |        |        |        |          |        |
| `Str`    |   X   |   X    |   X    |        |   X   |    X    |     X    |       |        |   X    |   X    |          |        |
| `Array`  |       |        |   X    |        |   X   |    X    |     X    |       |        |   X    |        |          |        |
| `Vector` |       |        |   X    |        |   X   |    X    |     X    |       |        |   X    |        |          |        |
| `Map`    |       |        |   X    |        |   X   |         |          |   X   |        |   X    |        |          |        |
| `Func`   |       |        |   X    |        |   X   |         |          |       |    X   |        |        |          |        |
| `Iter`   |       |        |   X    |        |   X   |         |          |       |        |   X    |        |          |        |
| `Byte`   |       |        |   X    |        |   X   |         |          |       |        |        |   X    |          |        |
| `IOFile` |       |        |   X    |        |   X   |         |          |       |        |        |        |    X     |        |
| `Type`   |       |        |   X    |        |   X   |         |          |       |        |        |        |          |   X    |


| From | To |
|------|----|
|`Int` |`Str`|
|`Int` | `Bool`|
|`Int` |`Real`|
|`Int` | `Byte`|
|`Real`|`Str`|
|`Real`|`Bool`|
|`Real`|`Int`|
|`Bool`|`Str`|
|`Type`|`Str`|
|`Type`|`Bool`|
|`Byte`|`Str`|
|`Byte`|`Bool`|
|`Byte`|`Int`|
|`Byte`|`Real`|

`@` takes the number of arguments of the function plus the function itself as
the last argument.

Here the function `func` is called with three arguments:

```text
1 2 3 @func
```

`*@` calls a function as well but you can have the arguments inside a vector or
an array:

```text
{ 1, 2, 3 } *@func
```

`->` takes two or three arguments, the first argument is optional and is the
step, the second is where the range should start, and the last is where the
range should end.

This creates a range of even numbers from 10 (included) to 20 (excluded):

```text
2 10 -> 20
```

`!!` takes two arguments, like `::`. The first one is the name of the error and
the last one the message that is printed along with the error.

Here is what would be printed when using the operator in `example.nest`.

This is the file:

```text
-- file example.nest
'This Is The Name' !! 'this is the message'
```

This is the output of the program:

```text
> nest example.nest

File "example.nest" at line 2:
 2 | 'This Is The Name' !! 'this is the message'
This Is The Name - this is the message
```

### If expression

The syntax of the if expression is the following:

```text
condition ?
    -- condition is true
:
    -- condition is false
```

The result of the if expression is the result of the expression evaluated, so
`a b == ? 2 : 3` means that if a and b are equal, the expression evaluates to
`2` and to `3` otherwise. When there is no 'else' clause, the expression
evaluates to `null`.

If you want to execute multiple expressions or statements you can use brackets:

```text
a b == ? [
    -- code block here
] : [
    -- other code block here
]
```

When using a code block expression evaluates to `null`:

```text
(true ? [ true ] : false) = val
```

Here `val` is set to `null` because `true` is inside a block.

### Assignment expressions

An assignment expression assigns a value to a variable or changes a value in
a vector, map or array.

To write an assignment expression you write the value, followed by an equal sign
(`=`) or a compound equal sign (`+=`, `-=` or any stack operator which is not a
comparison operator) and ending with the name of the variable or an access to
an element in an array, vector or map.

```text
10 = a -- assigns the value 10 to the variable 'a'
```

---

There is another kind of assignment in Nest: the unpacking assignment.  
An unpacking assignment takes a vector or an array and splits its contents into
the variables.

```text
{ 1, 2 } = { a, b } --> now 'a' is 1 and 'b' is 2
```

It can also be nested:

```text
{ 1, { 2, 3 } } = { a, { b, c } } --> now 'a' is 1, 'b' is 2 and 'c' is 3
```

And can be used in for-as loops:

```text
|#| 'stditutil.nest' = itu
{ 'a', 'b', 'c' } = arr

... arr @itu.enumerate := { idx, ch } [
    >>> (idx ' ' ch '\n' ><)
]
```

this program outputs:

```text
1 a
2 b
3 c
```

### Access operator

The access operator `.` can be used to get a specific value from vectors, arrays,
strings and maps.

To access a value you write the value to access from, a dot, and the index of
the value to get.

---

To index arrays, vectors or strings you can use integers. The first element is
at index `0`, the second at index `1` etc. until `n - 1` where `n` is the length
of the array or vector.

You can use also negative integers where `-1` is the last element, `-2` is the
second to last element etc.

```text
{ 1, 2, 3 } = arr
arr.0 --> 1
arr. -1 --> 3
```

If you need to index based on a variable you must use parenthesis otherwise the
variable name is treated like a string.

```text
{ 1, 2, 3 } = arr
2 = idx

arr.idx --> ERROR: arr.idx is equal to arr.'idx'
arr.(idx) --> 3
```

To index multiple-dimension arrays or vectors you cannot chain multiple
extractions since a real number literal would be formed. To prevent this you can
either put parenthesis around the integer or put a space before the dot.

```text
{{ 1, 2 },
 { 3, 4 }} = arr

arr.(0).(1) --> 2
arr.1 .1 --> 4
arr.1.0 --> ERROR: cannot index an array with '1.0'
```

---

To index a map you put the key of the value after the dot. If a key is a string
that is also a valid variable name you can put the name without any quotes.

```text
{ 'key_1': 2, 'invalid var': 10 } = map

map.key_1 --> 10
map.invalid var --> ERROR: will try to get map.'invalid'
map.'invalid var' --> 10

map.not_a_key --> null
```

> NOTE: if you try to index a key that is not in the map the result will be
> `null`

## Statements

### The for loop

The for loop has the following syntax:

```text
... times_to_repeat [
    -- code
]
```

The for loop only repeats a block of code a certain number of times, to use an
iterator and assign a variable, use a for-as loop.

### The for-as loop

The for-as loop has the following syntax:

```text
... iterator := var_name [
    -- code
]
```

`iterator` is an expression that evaluates to an iterator and `var_name` is the
variable to be assigned.

This for loop prints the numbers one through ten:

```text
... 1->11 := i [
    >>> (i '\n' ><)
]
```

### The while and do-while loops

The while loop has the following syntax:

```text
?.. condition [
    -- code
]
```

The do-while loop has the following syntax:

```text
..? condition [
    -- code
]
```

The only difference between while and do-while loops is that do-while loops
execute the code once before checking the condition.

### Function declaration

A function declaration is a hash followed by the name of the function and the
name of its arguments:

```text
#func_name arg1 arg2 arg3 [
    -- code
]
```

When calling a function the arguments are taken from left to right:

```text
#print_args a b c [
    >>> (a ' ' b ' ' c '\n' ><)
]

1 2 3 @print_args --> outputs '1 2 3'
```

### The switch statement

The switch statement has the following syntax:

```text
|> expression [
    ? case_1 [
        -- code executed if expression is equal to case_1
    ]
    ? case_2 [
        -- code executed if expression is equal to case_2
    ]
    ? [
        -- code for the default case
    ]
]
```

If `expression` is not equal to any of the cases, the code for the default case
is executed. If there is no default case, nothing happens.  
When the code for a case is executed, the switch statement ends. To have the
same behaviour as C when the `break` keyword is omitted, you can use the `..`
keyword.

The following two codes are equivalent:

```c
// C / C++
switch 10
{
case 5:
    printf("5");
case 10:
    printf("10");
default:
    printf("default");
}
```

```text
-- Nest
|> 10 [
    ? 5 [
        >>> 5
        ..
    ]
    ? 10 [
        >>> 10
        ..
    ]
    ? [
        >>> 'default'
    ]
]
```

### The try-catch statement

The try-catch statement has the following syntax:

```text
??
    -- try block
?! error_var
    -- catch block
```

If you want multiple statements you can use brackets:

```text
?? [
    -- try block
] ?! error_var [
    -- catch block
]
```

When using a try-catch statement any error that occurs inside the try block will
be caught in the catch block; you cannot catch only specific errors.

If an error occurs, it will be stored inside `error_var` as a map with two keys:

- `name`: the name of the error
- `message`: the message of the error

> NOTE: you can call `error_var` with any valid variable name.

If you want to also get the position and traceback of the error you can use the
`try` function of the `stderr` library.
