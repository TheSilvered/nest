# General Nest syntax

## Comments

In Nest there are two kinds of comments: line comments and block comments.
Line comments start with `--` and end at the end of the line. If there is a
backslash at the end, the line feed is escaped and the line comment continues
to the next line.

Block comments start with `-/` and end with `/-`

```nest
-- This is a single line comment
-- This is another single line comment \
   that has the line-feed escaped

-/ this is a block comment which
   spans multiple lines without
   any escapes /-
```

### File arguments

In the command line there are some arguments that change the way a Nest file is
compiled and interpreted. You can add some of those argument specifying them
on the first line of the file. The line must start with `--$` and then you can
have any combination of the following space-separated arguments:

- `-O0` through `-O3` to specify the optimization level of the file; if the
  specified level is higher than the one specified by the actual command, the
  latter is used
- `--encoding=<name>` forces the file to be read with the given encoding, the
  list of valid encodings can be found [here](stdlib/io_library.md#open)
- `--no-default` will not add any [predefined variable](#predefined-variables)
  to the global scope except for `_vars_`

In this example the file will be read using CP1252, only simple expressions are
optimized and no predefined variables are added.

```nest
--$ --no-default --encoding=cp1252 -O1
```

!!!note
    Any invalid argument is ignored and when specifying various optimization
    levels only the last one is considered.

## Value literals

### Numeric literals

Integer literals can be decimal, binary, octal or hexadecimal. The first one has
no prefix, the second `0b`, the third `0o` and the fourth `0x`.

Binary and octal literals cannot be directly followed by another integer and
hexadecimal literals cannot be followed by a word.

Real number literals support scientific notation but must always have some
digits both before and after the decimal point.

Both integer and real literals can be prefixed with a minus `-` to make them
negative. There can also be a plus sign `+` before the number but that does not
change its sign.

```nest
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
3e10 -- invalid because real literals must always contain the decimal point
```

Nest has also a byte type that can be written as an integer literal followed by
a lowercase `b` or an uppercase one `B`. If the integer is written with an
hexadecimal literal, since adding a `b` at the end would be counted as digit,
you start with `0h` instead.

```nest
-- Byte literals
10b
256b -- equivalent to 0b
0b101b
0o123b
0hff
```

Decimal, binary and octal byte literals can be followed by a word or by another
literal but that could cause confusion, so it is better to keep a space.

```nest
10bab -- split into '10b' and 'ab'
0b0b0 -- split into '0b' (from '0b0b') and '0'
00b10 -- split into '0b' (from '00b') and '10'

-- these are much clearer
10b ab
0b0b 0
00b 10
```

### String literals

String literals begin and end with either single or double quotes but there
is a difference between the two: strings with double quotes can span multiple
lines.

To escape a character you can use a backslash `\` before the character and here
are all the valid escape sequences:

| Sequence   | Hex Value | Name                      |
|:-----------|:---------:|:--------------------------|
| `\\`       | `5c`      | Backslash                 |
| `\'`       | `27`      | Single quote              |
| `\"`       | `22`      | Double quotes             |
| `\a`       | `07`      | Alert / Bell              |
| `\b`       | `08`      | Backspace                 |
| `\e`       | `1b`      | Backspace                 |
| `\f`       | `0c`      | Form feed / Page break    |
| `\n`       | `0a`      | Line feed / Newline       |
| `\r`       | `0d`      | Carriage return           |
| `\t`       | `09`      | Horizontal tab            |
| `\v`       | `0b`      | Vertical tab              |
| `\xhh`     | -         | Hexadecimal byte          |
| `\ooo`     | -         | Octal byte                |
| `\uhhhh`   | -         | Any BMP Unicode character |
| `\Uhhhhhh` | -         | Any Unicode character     |
| `\(...)`   | -         | A Nest expression         |

!!!note
    `h` represents a hexadecimal byte (`0-9`, `a-f` or `A-F`) and `o` an
    octal one (`0-7`).

!!!note
    Octal escapes can have either one, two or three digits: `\0`, `\12` and
    `\012` are all valid.

!!!note
    `...` are to be substituted with the expression to evaluate.

### Array literals

Array literals start and end with curly braces (`{` and `}`) and contain various
expressions separated by a comma.

```nest
{1, 2, 3, 4}
```

!!!note
    Writing `{}` creates an empty map, not an empty array. To create an
    empty array you can write `{,}`.

You can also create an array with all of the same value like this:

```nest
{10;30} --> array of length 30, with all values that are 10
```

This puts the same object in all slots of the array meaning that if an
object can change values inside of itself (array, vector or map) the values of
all objects will change:

```nest
{{1, 2};2} = a --> 'a' is equal to {{1, 2}, {1, 2}}
3 = a.0 .0 --> 'a' is now equal to {{3, 2}, {3, 2}}
```

### Vector literals

Vector literals start with `<{` and end with `}>` and inside have various
expressions separated by a comma.

```nest
<{1, 2, 3, 4}>
```

Just like arrays, vectors also have a smaller syntax to fill all slots with the
same value:

```nest
<{10;30}> --> vector of length 30, with all values that are 10
```

### Map literals

Map literals start with `{` and end with `}` and inside have key-value pairs.

```nest
{'key_1': 1, 'key_2': 2}
```

The keys can be of type `Str`, `Int` or `Byte` since only these three types are
hashable. `Real` objects cannot be hashed because of floating point error.

### Anonymous functions (lambdas)

Lambdas are declared by using `##` followed by any number of identifiers that
are the names of the parameters.

A lambda can return the value of a single expression by following the parameters
with `=>` or contain multiple statements with a block of code, delimited by `[`
and `]`.

```nest
##a b => a b + --> this lambda adds 'a' and 'b', returning their value

-- This lambda also prints the operation
##a b [
    >>> (a ' + ' b '\n' ><)
    => a b +
]
```

See also [function declarations](#function-declaration) and the
[return statement](#the-return-statement).

## Predefined variables

- `Int`: integer type
- `Real`: real number type
- `Bool`: boolean type
- `Null`: null type
- `Str`: string type
- `Array`: array type, non-resizable ordered collection of objects
- `Vector`: vector type, resizable ordered collection of objects
- `Map`: hash map type, holds key-value pairs
- `Func`: function type
- `Iter`: iterator type
- `Byte`: byte type, an integer from 0 to 255
- `IOFile`: file type, similar to `FILE *` in C
- `Type`: type of all types
- `true`: boolean true
- `false`: boolean false
- `null`: the only possible value of type `Null`
- `_args_`: an array with the arguments passed in the command line
- `_vars_`: a table containing the variables of the local scope
- `_globals_`: a table containing the variables of the global scope

!!!note
    The value of `_globals_` in the global scope is `null`.

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
- `%` modulus
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
- `==` equal to
- `!=` not equal to
- `<.>` contains

To perform an operation with these operators you can write all the operands
followed by the operator:

```nest
1 2 3 +
2 8 ^
```

To prioritize certain operations or to limit the number of operands of an
operator, you can use parenthesis:

```nest
(3 2 ^) (4 2 ^) + 0.5 ^
```

this expression is equal to the python expression `(3**2 + 4**2) ** 0.5`.

The comparison operators (`<`, `<=`, `>`, `>=`, `==` and `!=`) operate in a
different way though. They check each pair of adjacent operands and then check
all the results to see if they are all true.

The following two expressions are equal but the first one is more concise:

```nest
1 2 3 4 <
(1 2 <) (2 3 <) (3 4 <) &&
```

!!!note
    In the expression above, the values `2` and `3` are not actually evaluated
    twice. If you had for example a function such as `#f [>>> 'hi\n'[] => 2]`
    and used it inside the expression `1 @@f 2 <`, `hi` would be only printed
    once.

!!!note
    Writing `a b c !=` does not mean that all three values are different
    from each other, it only means that adjacent ones are.

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

```nest
>>> (<{1, 2, 3}> 2 + '\n' ><) --> <{1, 2, 3, 2}>
>>> (<{1, 2, 3}> 2 - '\n' ><) --> <{1, 3}>
>>> (<{1, 2, 3}> 2 * '\n' ><) --> <{1, 2, 3, 1, 2, 3}>
>>> (<{1, 2, 3}> 2 / '\n' ><) --> 2
```

The operator `-` behaves differently with maps too. It will remove a key from a
map and return the map itself.

```nest
{'a': 1, 'b': 2} = m
>>> (m 'a' - '\n' ><) --> {'b': 2}
>>> (m 'j' - '\n' ><) --> {'b': 2} removing a key that does not exist does not
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

```nest
>>> 'Hello, world!\n'
```

This kind of operators have the highest precedence, this means that writing
`>>> 'Hello ' name '!\n' ><` will be executed as `(>>> 'Hello ') name '!\n' ><`,
to change the order you can use parenthesis: `>>> ('Hello ' name '!\n' ><)`

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

---

`::` only takes two arguments: the object to cast and the type to cast it to.

Here the number `10` is casted to a `Byte` object:

```nest
Byte :: 10
```

The valid type casts in Nest are the following:

| ↱            | `Str` | `Bool` | `Int` | `Real` | `Byte` | `Iter` | `Array` | `Vector` | `Map` | `Func` | `IOFile` | `Type` | `Null` |
|:------------:|:-----:|:------:|:-----:|:------:|:------:|:------:|:-------:|:--------:|:-----:|:------:|:--------:|:------:|:------:|
| **`Str`**    |   ✓   |    ✓   |   ✓   |   ✓    |    ✓   |    ✓   |    ✓    |     ✓    |       |        |          |        |        |
| **`Bool`**   |   ✓   |    ✓   |       |        |        |        |         |          |       |        |          |        |        |
| **`Int`**    |   ✓   |    ✓   |   ✓   |   ✓    |    ✓   |        |         |          |       |        |          |        |        |
| **`Real`**   |   ✓   |    ✓   |   ✓   |   ✓    |    ✓   |        |         |          |       |        |          |        |        |
| **`Byte`**   |   ✓   |    ✓   |   ✓   |   ✓    |    ✓   |        |         |          |       |        |          |        |        |
| **`Iter`**   |   ✓   |    ✓   |       |        |        |    ✓   |    ✓    |     ✓    |   ✓   |        |          |        |        |
| **`Array`**  |   ✓   |    ✓   |       |        |        |    ✓   |    ✓    |     ✓    |   ✓   |        |          |        |        |
| **`Vector`** |   ✓   |    ✓   |       |        |        |    ✓   |    ✓    |     ✓    |   ✓   |        |          |        |        |
| **`Map`**    |   ✓   |    ✓   |       |        |        |    ✓   |    ✓    |     ✓    |   ✓   |        |          |        |        |
| **`Func`**   |   ✓   |    ✓   |       |        |        |        |         |          |       |    ✓   |          |        |        |
| **`IOFile`** |   ✓   |    ✓   |       |        |        |        |         |          |       |        |     ✓    |        |        |
| **`Type`**   |   ✓   |    ✓   |       |        |        |        |         |          |       |        |          |    ✓   |        |
| **`Null`**   |   ✓   |    ✓   |       |        |        |        |         |          |       |        |          |        |    ✓   |

When `Int`, `Real` or `Byte` objects are casted to `Bool`, they become `false`
if they are zero and `true` otherwise.
When `Str`, `Array`, `Vector` or `Map` objects are casted to a boolean, it
returns `false` if their length zero and `true` otherwise.
When an `IOFile` is casted to `Bool`, it returns `true` if the file is open and
`false` if it has been closed.
When `Null` is casted to `Bool`, it always returns `false`.
When any other object is casted to a `Bool`, it always returns `true`.

---

`@` takes the number of arguments of the function plus the function itself as
the last argument.

Here the function `func` is called with three arguments:

```nest
1 2 3 @func
```

`*@` calls a function as well but you can have the arguments inside a vector or
an array:

```nest
{1, 2, 3} *@func
```

Both for `@` and `*@`, if there are less arguments than the function expected,
to the remaining ones is passed `null`.

---

`->` takes two or three arguments, the first argument is optional and is the
step, the second is where the range should start, and the last is where the
range should end.

This creates a range of even numbers from 10 (included) to 20 (excluded):

```nest
2 10 -> 20
```

---

`!!` takes two arguments, like `::`. The first one is the name of the error and
the last one the message that is printed along with the error.

Here is what would be printed when using this operator in `example.nest`.

```nest
'This Is The Name' !! 'this is the message'
```

Output:

```nest
> nest example.nest

File "example.nest" at line 1:
 1 | 'This Is The Name' !! 'this is the message'
This Is The Name - this is the message
```

### If expression

The syntax of the if expression is the following:

```nest
condition ?
    -- condition is true
:
    -- condition is false
```

This follows the rules of the ternary operator in other languages with the
exception that it does not require an 'else' branch and that the code inside
the branches can be both an expression and a statement.

When an if expression is missing the 'else' branch it is written like so:

```nest
condition ?
    -- body if true
```

In case there is no 'else' branch and the condition is `false`, the expression
evaluates to `null`, otherwise, if the condition is `true`, it evaluates to
the value of the body of the first branch.

If the body of a branch is a statement the expression will evaluate to `null`.

```nest
1 2 == ? 5 : 2 = var_1 --> var_1 will be equal to 2
1 1 == ? 5 : 2 = var_2 --> var_2 will be equal to 5

(1 2 == ? 5) = var_3 --> var_3 will be equal to null
(1 1 == ? 5) = var_4 --> var_4 will be equal to 5

1 2 == ? [5] : 2 = var_5 --> var_5 will be equal to 2
1 1 == ? [5] : 2 = var_6 --> var_6 will be equal to null
1 2 == ? 5 : [2] = var_7 --> var_7 will be equal to null
1 1 == ? 5 : [2] = var_8 --> var_8 will be equal to 5
```

### Assignment expressions

An assignment expression assigns a value to a variable or changes a value in
a vector, map or array.

To write an assignment expression you write the value, followed by an equal sign
(`=`) or a compound equal sign (`+=`, `-=` or any stack operator which is not a
comparison operator) and end with the name of the variable or an access to an
element in an array, vector or map.

```nest
10 = a -- assigns the value 10 to the variable 'a'
3 -= a -- decrements 'a' by 3, now it is 7
```

---

There is another kind of assignment in Nest: the unpacking assignment.
An unpacking assignment takes a vector or an array and splits its contents into
the variables.

```nest
{1, 2} = {a, b} --> now 'a' is 1 and 'b' is 2
```

It can also be nested:

```nest
{1, {2, 3}} = {a, {b, c}} --> now 'a' is 1, 'b' is 2 and 'c' is 3
```

And can be used in for-as loops:

```nest
|#| 'stditutil.nest' = itu
{'a', 'b', 'c'} = arr

... arr @itu.enumerate := {idx, ch} [
    >>> (idx ' ' ch '\n' ><)
]
```

This program outputs:

```nest
1 a
2 b
3 c
```

!!!note
    When using this assignment you cannot use compound assignments.

### Access operator

The access operator `.` can be used to get a specific value from vectors,
arrays, strings and maps.

To access a value you write the value to access from, a dot, and the index of
the value to get.

---

To index arrays, vectors or strings you can use integers. The first element is
at index `0`, the second at index `1` etc. until `n - 1` where `n` is the length
of the array or vector.

You can use also negative integers where `-1` is the last element, `-2` is the
second to last element etc.

```nest
{1, 2, 3} = arr
arr.0 --> 1
arr. -1 --> 3
```

If you need to index based on a variable you must use parenthesis otherwise the
variable name is treated like a string.

```nest
{1, 2, 3} = arr
2 = idx

arr.idx --> ERROR: arr.idx is equal to arr.'idx'
arr.(idx) --> 3
```

To index multiple-dimension arrays or vectors you cannot chain multiple
extractions since a real number literal would be formed. To prevent this you can
either put parenthesis around the integer or put a space before the dot.

```nest
{{1, 2},
 {3, 4}} = arr

arr.(0).(1) --> 2
arr.1 .1 --> 4
arr.1.0 --> ERROR: cannot index an array with '1.0'
```

---

To index a map you put the key of the value after the dot. If a key is a string
that is also a valid variable name you can put the name without any quotes.

```nest
{'key_1': 2, 'invalid var': 10} = map

map.key_1 --> 10
map.invalid var --> ERROR: will try to get map.'invalid'
map.'invalid var' --> 10

map.not_a_key --> null
```

!!!note
    If you try to index a key that is not in the map the result will be `null`.

## Statements

### The for loop

The for loop has the following syntax:

```nest
... times_to_repeat [
    -- code
]
```

The for loop only repeats a block of code a certain number of times, to use an
iterator and assign a variable, use a for-as loop.

### The for-as loop

The for-as loop has the following syntax:

```nest
... iterator := var_name [
    -- code
]
```

`iterator` is an expression that evaluates to an iterator and `var_name` is the
variable to be assigned.

This for loop prints the numbers one through ten:

```nest
... 1 -> 11 := i [
    >>> (i '\n' ><)
]
```

### The while and do-while loops

The while loop has the following syntax:

```nest
?.. condition [
    -- code
]
```

The do-while loop has the following syntax:

```nest
..? condition [
    -- code
]
```

The only difference between while and do-while loops is that do-while loops
execute the code once before checking the condition.

### Function declaration

A function declaration is a hash followed by the name of the function and the
name of its arguments:

```nest
#func_name arg1 arg2 arg3 [
    -- code
]
```

As with [lambdas](#anonymous-functions-lambdas), functions can also contain only
one expression and return its value:

```nest
#func_name arg1 arg2 => -/ expression /-
```

When calling a function the arguments are taken from left to right:

```nest
#print_args a b c [
    >>> (a ' ' b ' ' c '\n' ><)
]

1 2 3 @print_args --> outputs '1 2 3'
```

### The return statement

The return statement is introduced by `=>` and exits early from a function
returning the value of the expression that follows it.
If it is not followed by an expression or there is no such statement in the
function, `null` is returned.

### The switch statement

The switch statement has the following syntax:

```nest
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

The following two code snippets are equivalent:

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

```nest
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

```nest
??
    -- try block
?! error_var
    -- catch block
```

If you want multiple statements you can use brackets:

```nest
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

!!!note
    You can replace `error_var` with any valid variable name.

If you want to also get the position and traceback of the error you can use the
`try` function of the `stderr` library.
