# A quick tour

## Hello, World!

This is one of the first programs everyone writes when learning a new language
and in Nest it is written as

```nest
>>> 'Hello, World!\n'
```

though a more complete version might be

```nest
|#| 'std.nest' = std

'Hello, World!' @std.io.println
```

## Numbers and arithmetic operations

In Nest operate in a lisp-like way but with a different syntax, this program
adds the numbers `1`, `2` and `3` and prints them to the standard output:

```nest
|#| 'std.nest' = std

1 2 3 + @std.io.println
```

An operator that takes all the values that preceed it is called a 'stack
operator' because you can think of the values as being pushed on a stack and
the operator clearing them and pushing the result back. Other stack operators
include

- `*` for multiplication,
- `-` for subtraction,
- `/` for division,
- `^` for exponentation and
- `==` for equality.

!!!note
    Check the full list of stack operators in
    [general syntax](syntax/#stack-operators)

You can limit the amount of arguments an operator consumes using parenthesis:

```nest
|#| 'std.nest' = std

(2 2 ^) (2 2 *) == @std.io.println
```

This program will output `true`.

This expression is normally written as `2^2 == 2*2` (or `2**2 + 2*2`) and
strictly speaking the first pair of parenthesis is redundant but it makes the
core more readable in my opinion.

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
    [general syntax](syntax/#local-operators)

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

In Nest there are multiple collection types.

### Strings

Strings are a sequence of Unicode characters. They are ordered and immutable.

To write a string you can use both single quotes (`'`) and double quotes (`"`).

```nest
'This is a string'
"This is also a string"
```

Strings support various escape sequences such as `\n` and `\t` and also value
interpolation with `\(...)` where instead of `...` there is an expression.

```nest
|#| 'std.nest' = std

'World' = var
'Hello, \(var)!' @std.io.println  --> prints Hello, World!
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

To append one or more values to a vector you can use the `+` operator and `/`
pops them.

```nest
|#| 'std.nest' = std

<{}> = vec
vec @std.io.println --> prints <{}>
vec 2 3 +
vec @std.io.println --> prints <{2, 3}>
```

### Hash maps

Hash maps contain key-value pairs with unique keys. They are ordered.

```nest
{'first': 1, 2: 'second'}
```

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
