# Using `generate_docs.nest`

Inside the `tools/` folder in the root directory, one of the files is called
`generate_docs.nest`. This tool is used to generate the Nest documentation from
header files.

## Documentation comments

Documentation comments can be of two kinds: one-line, for simple definitions
and multi-line for more complex ones, though they all use the multi-line
comment syntax.

```c
/* This is a single-line doc comment */

/**
 * This is a multi line doc comment.
 */
```

With single-line comments you can only generate a brief description of the
declaration, with multi-line comments you can do much more.

## Multi-line comments

Multi-line comments always begin with `/**` and each line after that must start
with ` *` until the last line which must end with `*/`. And all text is split
into blocks.

A block begins either where the comment begins or where the previous block
ended and ends when the comment ends or a tag is found. Tags are used to
categorize each block and a comment with no tag is considered to have a `null`
one.

The currently supported tags are `brief`, `param` and `return` and are
introduced by an at sign (`@`).

### `null` blocks

Each block has its uses and its purpose. `null` blocks are used to give a quick
description and do nothing special. They are written under the `Description`
section before `brief` blocks in the order in which they appear, though it is
better to use only one.

### `brief` blocks

`brief` blocks are also written under the `Description` section after `null`
blocks in the order in which they appear. These blocks, with a special start,
can also create `Note` and `Warning` notices for the declaration they describe.

To create these notices the line should start with either ` * @brief Note:` or
` * @brief Warning:`. The letter after the colon is automatically capitalized.

### `param` blocks

`param` blocks fall under a special section which is given different names
depending on the type of declaration the document. It is called `Fields` for
structs, `Variants` for enums and unions, and `Parameters` for functions,
macros and typedefs. Their syntax is also particular, the first word is the
name of the parameter which must be followed by a colon and a space with only
then the description of the parameter. These blocks are displayed in an
unordered list.

### `return` blocks

`return` blocks are written inside the `Returns` section in the order they
appear.

### Lists in blocks

You can add an unordered list to the end of all block types, except for `param`.
This is done by ending a line with a colon and having the next one start with
` *! `. You cannot, however, continue the normal block text after starting the
unordered list but you can make another one.

### Full example

Documentation comment:

```c
/**
 * This is a short description.
 *
 * @brief This is a longer description that explains the functionality of the
 * function.
 *
 * @brief Note: this is a notice for this function.
 *
 * @brief Warning: this is a warning for this function.
 *
 * @param p1: the first parameter of the function
 * @param p2: the second parameter of the function
 * @param ...: variable parameters for the function
 *
 * @return This function returns these values:
 *! value 1,
 *! value 2 or
 *! value 3
 *
 * @return and does not throw an error.
 */
NstEXP i32 NstC Nst_some_random_function(int p1, int p2, ...);
```

This comment generates this markdown:

```text
### `Nst_some_random_function`

**Synopsis:**

\```better-c
i32 Nst_some_random_function(int p1, int p2, ...)
\```

**Description:**

This is a short description.

This is a longer description that explains the functionality of the function.

!!!note
    This is a notice for this function.

!!!warning
    This is a warning for this function.

**Parameters:**

- `p1`: the first parameter of the function
- `p2`: the second parameter of the function
- `...`: variable parameters for the function

**Returns:**

This function returns these values:

- value 1,
- value 2 or
- value 3

and does not throw an error.
```

!!! note
    The backslash before the triple back ticks is not in the generated markdown,
    but I added it to make it not close the code block.

## Links and images

With this tool you can also have links inside your documentation. Any text
inside back ticks `` ` `` is checked to create a link. If it matches a name
that is documented a link to that is generated, this ignores trailing spaces,
asterisks and parentheses, so `Nst_Obj` and `Nst_Obj *` will both link to the
same declaration. You can also add custom links to the text by creating a
single-line doc comment that does not actually document anything but creates
a custom link. This is the syntax for it:

```c
/* [docs:link name url type] */
```

`name` is the text inside the back ticks, if it contains spaces use a
backslash, `hello\world` will match `` `hello world` ``.

`url` is the URL that will be assigned to the link. This is normally just the
name of another symbol in the documentation. If you wish to use a normal URL
you can surround it with angle brackets.

`type` is the type of the link and can be omitted in which case it defaults to
`c`:

- `t` will leave the link as plain text
- `c` will put the link inside an inline code block
- `i` will make the link an image, with the text being the alt-text

```c
// link to an internal name
/* [docs:link Nst_IO_SUCCESS Nst_IOResult] */

// link to an external URL
/* [docs:link image\of\a\wave <https://tinyurl.com/43y8kjze> i] */
```

### Link resolution

Links are resolved as following:

1. Normalize the name removing any trailing spaces, asterisks or parenthesis
2. Check if the symbol is ignored, if so no link is found
2. Check if the symbol appears in a user-defined link (with the `docs:link`
   directive), if so return the corresponding link
3. Check if the symbol exists in the documentation, if so return its link
4. Otherwise no link is found

## Other directives

There are other directives which modify the behaviour of this tool.

### `ignore`

The `[docs:ignore]` directive, if found in the first line of a block will
ignore it entirely. This is useful if a macro is defined twice for different
platforms and the documentation is duplicated, as this tool does not allow for
duplicate symbols or when the block does not document any symbol.

```c
/** [docs:ignore]
This is a random multi-line comment that is being ignored completely
*/
```

### `ignore_sym`

The `[docs:ignore_sym name]` is similar to `docs:link` because it uses its own
doc comment and is used to ignore a symbol for linking. This is useful if a
symbol starting with `Nst` is not found in the defined ones and `generate_docs`
prints out a warning.

```c
// If inside a documentation comment `Nst_abc` is found no link is generated

/* [docs:ignore_sym Nst_abc] */
```

### `raw`

The `[docs:raw]` directive will parse the contents of a block as raw markdown
that is directly copied to the file. Similarly to `[docs:ignore]` it must be
placed in the first line of the comment.

```c
/* [docs:raw]
# This is a heading.

This is a paragraph.

And this is **bold text**.
*/
```
