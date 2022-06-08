# nest
A programming language with no keywords.

## Features
- Dynamically typed
- Memory safe
- Interpreted
- Function-oriented
- Has named variables
- Has hash maps, vectors and arrays

## Types
There are only the following types in Nest:
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

## Syntax

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
- `<<<`, in: prints the object and returns what the user typed in
- `?::`, type: gets the type of the object

The length operator works only on maps, arrays, vector and strings.

A program that asks for the user's name and greets him
```
<<< "What is your name?\n> " = name
>>> ("Hello " name "!\n" ><)
```
assignments and string escapes will be covered in other sections.

### Local/stack operations
This kind of operator affects the last of it's members differently from the
one preceding it but can take an indefinite amount of values.

The stack-local operators are:
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
here te function `add` is called with `1` and `2` as arguments.

---

`->` takes one or two arguments preceding it (start and step) and one after it
(stop).

```
0 2->10
```
this creates an iterator that starts at 0 and arrives ad 10 (not included) with
a step of two (giving `0`, `2`, `4`, `6`, `8`).

## Work in progress...