# General nest syntax

## Comments

In Nest comments start with a double hyphen `--` and end at the end of the line.
There are also multi-line comments that are either single-line comments which
have a single backslash `\` as their last character or block comments which
start with `-/` and end with `/-`.

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
-- Iteger literals
10
-123
012 -- equal to just 12 but still valid

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
| `\xhh`   | Hexadecimal byte, `hh` represents hex digits from `01` to `7f`  |

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

> NOTE: writing `a b c !=` does not mean that all three values are different, it
> only means that adjacent are.

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
