# General nest syntax

## Comments

In Nest there are two kinds of comments: line comments and block comments.
Line comments start with `--` and end at the end of the line. If there is a
backslash at the end, the line feed is escaped and the line comment continues
to the next line.

Block comments start with `-/` and end with `/-`

```
-- This is a single line comment
-- This is another single line comment \
   that has the line-feed escaped

-/ this is a block comment which
   spans multiple lines without
   any escapes /-
```

## Value literals

### Numeric literals

There are no binary, octal or hexadecimal integer literals so the only possible
integer is decimal.

Real number literals do not support scientific notation and must have some
digits both before and after the dot.

Both integer and real literals can be prefixed with a minus `-` to make them
negative.

```
-- Integer literals
10
-123
012 -- equal to 12 but still valid

-- Real literals
0.2
-13.4
1.0
1. -- invalid
.3 -- also invalid
```

### String literals

String literals begin and end with either a single or a double quote but there
is a difference between the two: strings with double quotes can span multiple
lines.

To escape a character you can use a backslash `\` before the character and here
are all the valid escape sequences:

| Sequence | Name                   |
|----------|------------------------|
| `\\`     | Backslash              |
| `\'`     | Single quote           |
| `\"`     | Double quotes          |
| `\a`     | Alert / Bell           |
| `\b`     | Backspace              |
| `\f`     | Form feed / Page break |
| `\n`     | Line feed / Newline    |
| `\t`     | Horizontal tab         |
| `\v`     | Vertical tab           |
| `\xhh`   | Hexadecimal byte, `hh` represents hex digits from `00` to `ff` |
| `\nnn`   | Octal byte, `nnn` represents octal digits from `0` to `777` |

> NOTE: octal escapes can have either one, two or three digits, `\12` and `\012`
> are both valid escapes

### Array literals

Array literals start and end with braces (`{` and `}`) and inside have various
expressions separated by a comma.

```
{ 1, 2, 3, 4 }
```

> NOTE: writing `{ }` creates an empty map, not an empty array. To create an
> empty array you can write `Array :: <{ }>`

You can also create an array with all of the same value like this:
```
{ 10;30 } --> array of length 30, with all values that are 10
```

This puts the same object in all slots of the array meaning that if an
object can change values inside of itself (array, vector or map) the values of
all objects will change:
```
{ { 1, 2 };2 } = a --> 'a' is equal to { { 1, 2 }, { 1, 2 } }
3 = a.0 .0 --> 'a' is now equal to { { 3, 2 }, { 3, 2 } }
```

### Vector literals

Vector literals start with `<{` and end with `}>` and inside have various
expressions separated by a comma.

```
<{ 1, 2, 3, 4 }>
```

Just like arrays, vectors also have a smaller syntax to fill all slots with the
same value:
```
<{ 10;30 }> --> vector of length 30, with all values that are 10
```

### Map literals

Map literals start with `{` and end with `}` and inside have key-value pairs.

```
{ 'key_1': 1, 'key_2': 2 }
```

### Anonymous functions (lambdas)

Lambdas are one-expression functions that have no name. Their syntax is the
following:

```
##arg1 arg2 arg3 => expression
```

If you want to store a lambda in a variable you need parenthesis otherwise the
assignment will be included in the expression returned.

```
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
- `+` addition
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
```
1 2 3 +
2 8 ^
```

To prioritize certain operations or to limit the number of operands of an
operator, you can use parenthesis:
```
(3 2 ^) (4 2 ^) + 0.5 ^
```
this expression is equal to the python expression `(3 ** 2 + 4 ** 2) ** 0.5`.

The comparison operators (`<`, `<=`, `>`, `>=`, `==` and `!=`) operate in a
different way though. They check each pair of adjacent operands and then check
all the results to see if they are all true.

The following two expressions are equal but the first one is more concise:
```
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
`true` if there was a value to remove and `false` otherwise.

The `*` operator repeats the contents of a vector a number of times and returns
the vector itself.

The `/` operator pops a number of values from the end and returns the last value
popped.

```
>>> (<{ 1, 2, 3 }> 2 + '\n' ><) --> <{ 1, 2, 3, 2 }>
>>> (<{ 1, 2, 3 }> 2 - '\n' ><) --> true
>>> (<{ 1, 2, 3 }> 2 * '\n' ><) --> <{ 1, 2, 3, 1, 2, 3 }>
>>> (<{ 1, 2, 3 }> 2 / '\n' ><) --> 2
```

The operator `-` behaves differently with maps too. It will remove a key from a
map and return `true` if the key existed, `false` otherwise.

```
{ 'a': 1, 'b': 2 } = m
>>> (m 'a' - '\n' ><) --> true
>>> (m 'j' - '\n' ><) --> false
>>> m --> { 'b': 2 }
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
```
>>> 'Hello, world!\n'
```

### Local-stack operators

This third type of operator in Nest can take a set number of arguments,
depending on the operator you are using. These operators are used do to specific
functions built in the language.

The local-stack operators are:
- `::` casts a type to another
- `@` calls a function
- `->` creates an integer iterator

To use a local-stack operator you write the last argument to the right and all
the others to the left.

`::` only takes two arguments: the object to cast and the type to cast it to.

Here the number `10` is casted to a `Byte` object:
```
Byte :: 10
```

`@` takes the number of arguments of the function plus the function itself as
the last argument.

Here the function `func` is called with three arguments:
```
1 2 3 @func
```

`->` takes two or three arguments, the first argument is where the range should
start, the second is optional and is the step, and the last is where the range
should end.

This creates a range of even numbers from 10 (included) to 20 (excluded):
```
10 2 -> 20
```

### If expression

The syntax of the if expression is the following:
```
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
```
a b == ? [
    -- code block here
] : [
    -- other code block here
]
```

When using code blocks the expression is always `null`.

### Assignment expressions

An assignment expression assigns a value to a variable or changes a value in
a vector, map or array.

To write an assignment expression you write the value, followed by an equal sign
(`=`) or a compound equal sign (`+=`, `-=` or any stack operator which is not a
comparison operator) and ending with the name of the variable or an access to
an element in an array, vector or map.

```
10 = a -- assigns the value 10 to the variable 'a'
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

```
{ 1, 2, 3 } = arr
arr.0 --> 1
arr. -1 --> 3
```

If you need to index based on a variable you must use parenthesis otherwise the
variable name is treated like a string.

```
{ 1, 2, 3 } = arr
2 = idx

arr.idx --> ERROR: arr.idx is equal to arr.'idx'
arr.(idx) --> 3
```

To index multiple-dimension arrays or vectors you cannot chain multiple
extractions since a real number literal would be formed. To prevent this you can
either put parenthesis around the integer or put a space before the dot.

```
{{ 1, 2 },
 { 3, 4 }} = arr

arr.(0).(1) --> 2
arr.1 .1 --> 4
arr.1.0 --> ERROR: cannot index an array with '1.0'
```

---

To index a map you put the key of the value after the dot. If a key is a string
that is also a valid variable name you can put the name without any quotes.

```
{ 'key_1': 2, 'invalid var': 10 } = map

map.key_1 --> 10
map.invalid var --> ERROR: will try to get map.'invalid'
map.'invalid var' --> 10

map.not_a_key --> null
```

> NOTE: if you try to index a key that is not in the map the result will be
> `null`

###

## Statements

### The for loop

The for loop has the following syntax:
```
... times_to_repeat [
    -- code
]
```

The for loop only repeats a block of code a certain number of times, to use an
iterator and assign a variable, use a for-as loop.

### The for-as loop

The for-as loop has the following syntax:
```
... iterator ~= var_name [
    -- code
]
```

`iterator` is an expression that evaluates to an iterator and `var_name` is the
variable to be assigned.

This for loop prints the numbers one through ten:
```
... 1->11 ~= i [
    >>> (i '\n' ><)
]
```

### The while and do-while loops

The while loop has the following syntax:
```
?.. condition [
    -- code
]
```

The do-while loop has the following syntax:
```
..? condition [
    -- code
]
```

The only difference between while and do-while loops is that do-while loops
execute the code once before checking the condition.

### Function declaration

A function declaration is a hash followed by the name of the function and the
name of its arguments:
```
#func_name arg1 arg2 arg3 [
    -- code
]
```

When calling a function the arguments are taken from left to right:
```
#print_args a b c [
    >>> (a ' ' b ' ' c '\n' ><)
]

1 2 3 @print_args --> outputs '1 2 3'
```

### The switch statement

The switch statement has the following syntax:
```
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

```
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
