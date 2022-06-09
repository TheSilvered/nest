# nest
A programming language with no keywords.

## Features
- Dynamically typed
- Memory safe
- Interpreted
- Function-oriented
- Has named variables
- Has hash maps, vectors and arrays

## Syntax

### Predefined variables
The built-in type names:
- `Int`, 64-bit integers
- `Real`, double precision floating point numbers
- `Bool`, boolean
- `Null`, absence of a value
- `Str`, ASCII string
- `Array`, array of objects (of any type)
- `Vector`, resizable array of objects
- `Map`, hash map
- `Func`, function
- `Iter`, iterator
- `Byte`, byte
- `IOfile`, file
- `Type`, any type's type

And values that are always the same object:
- `true`, boolean true
- `false`, boolean false
- `null`, null object, not to confuse with the type `Null`

### Comments

Comments in nest are started by `--` and end with the end of a line.
Multi-line comments are created by escaping the line feed at the end with a
backslash.

```
-- This is a comment

-- This is a comment that \
   spans multiple lines
```

Note that if there are any characters after `\` in multi-line comments, the
escape doesn't work.

### Stack operations

This kind of operations take one or more values and they do the operation to
every value preceding them and leaving one on the stack.

```
1 2 3 +
```
here the '+' stack operator adds 1, 2 and 3 giving a result of 6.

At the end of an expression there must be only one value on the stack, meaning
that this doesn't work.

```
3 3 - 2
```
here there are two values on the stack: 0 (3 - 3) and 2.

The stack operators are:
- `+`, addition
- `-`, subtraction
- `*`, multiplication
- `/`, division
- `^`, exponentiation (power)
- `%`, reminder
- `&&`, logical and
- `||`, logical or
- `&|`, logical xor
- `>`, minority
- `<`, majority
- `==`, equality
- `!=`, inequality
- `>=`, equality or majority
- `<=`, equality or minority
- `&`, bit-wise and
- `|`, bit-wise or
- `^^`, bit-wise xor
- `<<`, bit-wise left shift
- `>>`, bit-wise right shift
- `><`, concatenation (all values become strings and are merged)

Some stack operators do not behave like the others though.

All comparison operators (`>`, `<`, `==`, `!=`, `>=`, `<=`) compare each
adjacent values and check if all of them are true meaning that `1 2 3 <` yields 
the same result as `(1 2 <) (2 3 <) &&` and yes, you can use parenthesis to 
prioritize sub-expressions.
Keep in mind that writing `a b c !=` does not mean that all the elements are
different from each other, you only know that adjacent elements are different
meaning that a and c could be both 1 and b could be 2 and the expression would
be true.

### Local operations

This kind of operator affects the value immediately after it.

There local operators are:
- `$`, length: gets the length of the object
- `!`, logical not
- `~`, bit-wise not
- `>>>`, out: casts the object to a string and prints it to stdout
- `<<<`, in: prints the object and reads stdin
- `?::`, type: gets the type of the object

The length operator works only on maps, arrays, vector and strings.

A program that asks for the user's name and greets him
```
<<< "What is your name?\n> " = name
>>> ("Hello " name "!\n" ><)
```
assignments and string escapes will be covered in other sections.

### Local-stack operations
This kind of operator affects the last of it's members differently from the
one preceding it but can take an indefinite amount of values.

The local-stack operators are:
- `::`, cast: changes the type of an object
- `@`, call: calls a function
- `->`, range: creates an iterator with a range

`::` takes only one argument preceding it and one following, the first should be
the type to which the second is casted.

```
Real :: 10
```
this casts 10 (an `Int`) to a `Real`

---

`@` takes zero or more arguments preceding it, they are read in the same order
of the declaration, and one following which is the function

```
1 2 @add
```
here the function `add` is called with `1` and `2` as arguments.

---

`->` takes one or two arguments preceding it (start and step) and one after it
(stop).

```
0 2->10
```
this creates an iterator that starts at 0 and arrives ad 10 (not included) with
a step of two (giving `0`, `2`, `4`, `6`, `8`).

### Numbers

There are two types of numbers in Nest: integers and real numbers.

Integers can only be written in base 10 and no underscores (`_`) are allowed
between the numbers. (Should change in the future)

Real numbers are written with a dot and there must be digits on either side, so
`1.` and `.1` are both invalid but `1.0` or `0.1` are correct.
It's not possible to write them with scientific notation (such as `1.3e4`).

### Strings

Strings are written by enclosing parts of text with single (`'`) or double (`"`)
quotes.

Between these there is almost no difference, but the latter allows for
multi-line strings.

```
'This is a single-line string'
"This is a
 multi-line string"

'This is a
 syntax error'
```

There escapes supported are:

| Escape | Name                |
|--------|---------------------|
| `\'`   | single quote        |
| `\"`   | double quote        |
| `\\`   | backslash           |
| `\a`   | alert / bell        |
| `\b`   | backspace           |
| `\f`   | form feed           |
| `\n`   | newline / line feed |
| `\r`   | carriage return     |
| `\t`   | horizontal tab      |
| `\v`   | vertical tab        |

### Arrays

An array is an ordered sequence of objects that has always the same size.
It is written by writing its values inside curly braces and separating them
with commas.

```
{ 1, 2, 3, 4, 5 }
```
this is an array containing 1, 2, 3, 4 and 5 in this order.

To get a value from the array you can use the extraction operator (`.`) followed
by the index. Indices go from 0 to n - 1 where n is the length of the array.
You can use negative indices too and to get the actual index just add the length
of the array.

```
{ 1, 2, 3, 4, 5 } = arr
arr.4 -- last item
arr. -1 -- also last item
```
With the negative index a space is needed to separate the dot `.` and the hyphen
`-` because together create an nonexistent operator `.-`.

You cannot index an array directly with variable names, those need to be inside
parenthesis otherwise they will be treated like strings.

```
0 = idx
{ 1, 2, 3, 4, 5 } = arr

arr.idx -- error, this is equal to writing arr.'idx'
arr.(idx) -- does what is expected
```

### Vectors

Vectors are similar to arrays but they can also change size.
The syntax is similar too, but instead of using `{` and `}` as the delimiters,
you use `<{` and `}>`.

```
<{ 1, 2, 3, 4, 5 }>
```
this is a vector containing 1, 2, 3, 4 and 5 in this order.

Indexing a vector is identical to indexing an array.

The operators `+`, `-`, `*` and `/` each have a special function to operate
with the data in the vector.

The `+` operator adds an element to the end of the vector and returns the vector
itself.

```
<{ 1, 2 }> = v
v 3 +
>>> arr.2 --> 3
```
----

The `-` removes the firs occurrence of an element from the vector and returns
the vector itself.

```
<{ 1, 2 }> = v
arr 1 -
>>> arr.0 --> 2
```

----

The `*` operator repeats the contents of the vector and returns the vector
itself.

```
<{ 1, 2 }> = v
v 3 *
>>> arr.2 --> 1
>>> arr.3 --> 2
>>> arr.4 --> 1
```

----

The `/` operator drops a certain amount of items from a vector and returns the
last one dropped.

```
<{ 1, 2, 3 }> = v
>>> (v 2 /) --> 2

<{ 1, 2 }> = v
>>> (v 1 /) --> 3
```

### Maps

A map is a hash table that contains key-value pairs and that can change size.
To create a hash table you delimit the whole structure with curly braces and
inside there the keys followed by a semicolon and a value. Multiple key-value
pairs are separated with commas.

The types that can be used as keys are integers, strings, boolean, null and type.

```
{ 'key_1': 1,
  'key_2': 2,
  'key_3': 3 } = m
```

There cannot be two equal keys inside the map but two values with the same hash
can exist.

To index a map you can use the extraction operator followed by the key.

```
>>> m.key_1 --> 1, since m.key_1 and m.'key_1' are the same
```

### Functions

A function is declared with the hash symbol `#`. Following it there is the name
of the function and the name of its arguments and the body encapsulated by two
brackets.

```
#func arg1 arg2 arg3 [
    -- body
]
```

To return a value you can use `=>` followed by the value to return.

```
#add n1 n2 [
    => n1 n2 +
]
```

### While and do-while loops

A while loop has the while symbol `?..` followed by the condition and the body
encapsulated by two brackets.

```
?.. condition [
    -- body
]
```

Do-while loops are very similar except for the fact that they use the do-while
symbol `..?`

```
..? condition [
    -- body
]
```

To break out of the current loop you can use a semi-colon `;` and to stop the
current iteration and continue to the next one you can use two dots `..`

Any variable declared inside a loop does not get deleted when the loop ends but
is accessible in the local scope

```
?.. true [
    3 = a
    ;
]

>>> a --> 3
```

### If expressions

An if expression is written with a condition, followed by a question mark and
the code to execute if the condition is true. Optionally, you can add a colon
and set the code to execute when the condition is false.

```
condition ? >>> 'The condition is true\n' : >>> 'The condition is false\n'
```

To execute more than one instruction you can use a code block (code between two
brackets).

```
name ? [
    <<<'' = name
    >>> name
] : [
    >>> name
]
```

An if expression only returns a value with single-line statements, code blocks
always return null.

### Assignment expressions

To assign a value to a variable you type the value expression followed by an
equal sign and the name of the variable or the expression you would use to
access an item in a map, vector or array.

You can use assignments to add a key to a map.

```
1 = n
>>> n --> 1

{ 1, 2 } = a
3 = a.1
>>> a.1 --> 3

{ 1: 10, 2: 20 } = m
30 = m.3
200 = m.2
>>> m.3 --> 30
>>> m.2 --> 200
```

There are also some compound operators for syntactic sugar:
- `+=`, addition
- `-=`, subtraction
- `*=`, multiplication
- `/=`, division
- `^=`, exponentiation (power)
- `%=`, reminder
- `&=`, bit-wise and
- `|=`, bit-wise or
- `^^=`, bit-wise xor
- `<<=`, bit-wise left shift
- `>>=`, bit-wise right shift
- `><=`, concatenation

and they simply to the stack operation taking the new value and the variable's
value.

```
1 = a
>>> a --> 1

2 += a
>>> a --> 3
```

### Type casting

We have seen before the cast operator `::` but here are all the valid casts in
Nest.

| Initial type | Casted Type |
|--------------|-------------|
| Int          | Real        |
| Int          | Bool        |
| Int          | Str         |
| Real         | Int         |
| Real         | Bool        |
| Real         | Str         |
| Bool         | Str         |
| Null         | Bool        |
| Str          | Bool        |
| Str          | Iter        |
| Array        | Bool        |
| Array        | Iter        |
| Vector       | Bool        |
| Vector       | Iter        |
| Map          | Bool        |
| Map          | Iter        |
| Byte         | Int         |
| Byte         | Real        |
| Byte         | Bool        |
| Byte         | Str         |

Casting to a boolean `null`, it will always return `false`.
Casting to a boolean an array, vector, string or map will return `false` when
it's empty, otherwise true.
Casting to a boolean a number (Byte, Int or Real), it will return `false` when
it is equal to zero.
Casting
