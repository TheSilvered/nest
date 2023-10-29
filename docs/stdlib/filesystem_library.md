# Filesystem library

## Importing

```nest
|#| 'stdfs.nest' = fs
```

## Functions

### `@absolute_path`

**Synopsis:**

```nest
[path: Str] @absolute_path -> Str
```

**Arguments:**

- `path`: the relative path to expand

**Returns:**

The absolute path of a file or directory.

---

### `@canonical_path`

**Synopsis:**

```nest
[path: Str] @canonical_path -> Str
```

**Arguments:**

- `path`: the relative path to expand

**Returns:**

The canonical path of a file or directory. A canonical path is an absolute path
that resolves any links.

---

### `@copy`

**Synopsis:**

```nest
[from: Str, to: Str, options: Int?] @copy -> null
```

**Description:**

Copies a file or a directory from `from` to `to`. `options` must be set with
the [`CPO`](#cpo) constant and its members. If `options` is `null`, `CPO.none`
is used. An error is thrown if either the source or the destination path does
not exits or if a system error occurs.

**Arguments:**

- `from`: the path of the element to copy
- `to`: the path the element should be copied as
- `options`: the options that modify the default behaviour of copy

---

### `@equivalent`

**Synopsis:**

```nest
[path_1: Str, path_2: Str] @equivalent -> Bool
```

**Arguments:**

- `path_1`: the path of the first file
- `path_2`: the path of the second file

**Returns:**

`true` if the two paths point to the same file and `false` otherwise. This
returns `true` even when the two paths are different but are hard links to the
same file.

---

### `@exists`

**Synopsis:**

```nest
[path: Str] @exists -> Bool
```

**Returns:**

`true` if `path` points to an existing filesystem object.

---

### `@extension`

**Synopsis:**

```nest
[path: Str] @extension -> Str
```

**Arguments:**

- `path`: the path of a file

**Returns:**

The extension of the file pointed to by `path`.

**Example:**

```nest
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.extension --> '.txt'
'dir/subdir/file.tar.gz' @fs.extension --> '.gz'
'dir/subdir/file' @fs.extension --> ''
'dir/subdir/subdir2' @fs.extension --> ''
'dir/subdir/subdir2/' @fs.extension --> ''
```

---

### `@filename`

**Synopsis:**

```nest
[path: Str] @filename -> Str
```

**Arguments:**

- `path`: the path of a file

**Returns:**

The name of the file `path` points to.

**Example:**

```nest
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.filename --> 'file.txt'
'dir/subdir/subdir2'  @fs.filename --> 'subdir2'
'dir/subdir/subdir2/' @fs.filename --> ''
```

---

### `@is_block_device`

**Synopsis:**

```nest
[path: Str] @is_block_device -> Bool
```

**Arguments:**

- `path`: the path of the block device to check

**Returns:**

`true` if the path is a block device and `false` otherwise.

---

### `@is_char_device`

**Synopsis:**

```nest
[path: Str] @is_char_device -> Bool
```

**Arguments:**

- `path`: the path of the character device to check

**Returns:**

`true` if the path is a character device and `false` otherwise.

---

### `@is_dir`

**Synopsis:**

```nest
[path: Str] @is_dir -> Bool
```

**Description:**

Checks if a directory exists at `path`.

**Arguments:**

- `path`: the path of the directory to check

**Returns:**

`true` if the path is a directory and `false` otherwise.

---

### `@is_file`

**Synopsis:**

```nest
[path: Str] @is_file -> Bool
```

**Description:**

Checks if a file exists at `path`.

**Arguments:**

- `path`: the path of the file to check

**Returns:**

`true` if the path is a file and `false` otherwise.

---

### `@is_named_pipe`

**Synopsis:**

```nest
[path: Str] @is_named_pipe -> Bool
```

**Arguments:**

- `path`: the path of the FIFO to check

**Returns:**

`true` if the path is a FIFO and `false` otherwise.

---

### `@is_socket`

**Synopsis:**

```nest
[path: Str] @is_socket -> Bool
```

**Arguments:**

- `path`: the path of the socket to check

**Returns:**

`true` if the path is a socket and `false` otherwise.

---

### `@is_symlink`

**Synopsis:**

```nest
[path: Str] @is_symlink -> Bool
```

**Arguments:**

- `path`: the path of the symlink to check

**Returns:**

`true` if the path is a symlink and `false` otherwise.

---

### `@join`

**Synopsis:**

```nest
[path_1: Str, path_2: Str] @join -> Str
```

**Description:**

Joins two paths by adding, if needed, a slash between them.

**Arguments:**

- `path_1`: the first path
- `path_2`: the path to append to `path_1`

**Returns**:

Returns the two paths joint. If `path_2` is an absolute path it is returned
without any modifications. This function normalizes the slashes after joining:
on Windows `/` becomes `\` and on Linux `\` becomes `/`.

---

### `@list_dir`

**Synopsis:**

```nest
[path: Str] @list_dir -> Vector
```

**Description:**

Lists the contents of a directory, both files and sub-directories but not the
contents of the latter. Throws an error if `path` does not point to a valid
directory or a system error occurs.

**Arguments:**

- `path`: the directory of which the contents are listed

**Returns:**

A vector containing the paths of all the elements inside the directory, the
`path` given prepends each element.

---

### `@list_dirs`

**Synopsis:**

```nest
[path: Str] @list_dirs -> Vector
```

**Description:**

Similar to [`list_dir`](#list_dir) but also lists the contents of the
sub directories recursively. Throws an error if `path` does not point to a
valid directory or a system error occurs.

**Arguments:**

- `path`: the directory of which the contents are listed

**Returns:**

A vector containing the paths of all the elements inside the directory and its
sub directories, the `path` given prepends each element.

---

### `@make_dir`

**Synopsis:**

```nest
[path: Str] @make_dir -> null
```

**Description:**

Creates a new directory at `path`, succeeds even if the directory already
exists. An error is thrown if any parent directory does not exist or a system
error occurs.

**Arguments:**

- `path`: the path of the directory to create

---

### `@make_dir_symlink`

**Synopsis:**

```nest
[target: Str, link: Str] @make_dir_symlink -> null
```

**Description:**

Creates a new symbolic link that points to a directory.

**Arguments:**

- `target`: the directory the new symbolic link points to, does not have to
  exist
- `link`: the path of the new symbolic link

---

### `@make_dirs`

**Synopsis:**

```nest
[path: Str] @make_dirs -> null
```

**Description:**

Creates a directory at `path`, and the parent directories if needed, succeeds
even if the directory already exists. An error is thrown only if a system error
occurs.

**Arguments:**

- `path`: the path of the directories to create

---

### `@make_file_symlink`

**Synopsis:**

```nest
[target: Str, link: Str] @make_file_symlink -> null
```

**Description:**

Creates a new symbolic link that points to a file.

**Arguments:**

- `target`: the file the new symbolic link points to, does not have to exist
- `link`: the path of the new symbolic link

---

### `@make_hard_link`

**Synopsis:**

```nest
[target: Str, link: Str] @make_hard_link -> null
```

**Description:**

Creates a new hard link that points either to a file or a directory.

**Arguments:**

- `target`: the file or directory the new hard link points to
- `link`: the path of the new hard link

---

### `@normalize`

**Synopsis:**

```nest
[path: Str] @normalize -> Str
```

**Description:**

Translates any slashes in the path to the native separators. This means that on
Windows `/` becomes `\` and on Linux `\` becomes `/`.

**Returns:**

The normalized path.

!!!warning
    On Linux the file system allows for backslashes in names but this function
    will replace them. `dir1/dir2/file\.txt` becomes `dir1/dir2/file/.txt` even
    if the first is a valid path to a file called `file\.txt`.

---

### `@parent_path`

**Synopsis:**

```nest
[path: Str] @parent_path -> Str
```

**Arguments:**

- `path`: the path to a file or a directory

**Returns:**

The path of the directory where the element is contained.

**Example:**

```nest
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.parent_path --> 'dir/subdir'
'dir/subdir/subdir2'  @fs.parent_path --> 'dir/subdir'
'dir/subdir/subdir2/' @fs.parent_path --> 'dir/subdir/subdir2'
```

---

### `@read_symlink`

**Synopsis:**

```nest
[path: Str] @read_symlink -> Str
```

**Description:**

Reads the target of a symlink. Throws an error if the symlink does not exist or
a system error occurs.

**Arguments:**

- `path`: the path of the symlink to read

**Returns:**

The target of the symlink which may not exist.

---

### `@relative_path`

**Synopsis:**

```nest
[path: Str, base: Str] @relative_path -> Str
```

**Description:**

Returns a relative path to `path` using `base` as the starting point.

**Arguments:**

- `path`: the directory to reach
- `base`: the starting directory

**Example:**

```nest
|#| 'stdfs.nest' = fs

'/a/c' '/a/b/d' @fs.relative_path --> '../../c'
```

---

### `@remove_dir`

**Synopsis:**

```nest
[path: Str] @remove_dir -> null
```

**Description:**

Removes a directory at `path` which must be empty. An error is thrown if the
directory contains any sub directories or files, the path does not point to a
valid directory or if a system error occurs.

**Arguments:**

- `path`: the path of the directory to delete

---

### `@remove_dirs`

**Synopsis:**

```nest
[path: Str] @remove_dirs -> null
```

**Description:**

Removes a directory at `path` and all of its contents, throws an error if the
directory does not exist or a system error occurs.

**Arguments:**

- `path`: the path of the directory to delete

---

### `@remove_file`

**Synopsis:**

```nest
[path: Str] @remove_file -> null
```

**Description:**

Removes a file at `path`, throws an error if the file does not exist or a
system error occurs.

**Arguments:**

- `path`: the path of the file to delete

---

### `@rename`

**Synopsis:**

```nest
[old_path: Str, new_path: Str] @rename -> null
```

**Description:**

Renames or moves a file or a directory. Throws an error if `old_path` does not
exist or if a system error occurs.

**Arguments:**

- `old_path`: the old path of the element
- `new_path`: the path the element should be moved to

---

### `@_get_copy_options`

**Synopsis:**

```nest
[] @_get_copy_options -> Map
```

**Returns:**

A new [`CPO`](filesystem_library.md#cpo) map.

---

## Constants

### `CPO`

A map that has the copy options.
Its members are:

| Name              | Description                                                      |
| ----------------- | ---------------------------------------------------------------- |
| `none`            | Normal copy, follows symlinks and skips sub-directories          |
| `skip`            | Skips existing files                                             |
| `overwrite`       | Overwrites existing files                                        |
| `update`          | Overwrites existing files if they are older than the file copied |
| `recursive`       | Copies recursively also sub-directories                          |
| `copy_symlinks`   | Copies symlinks, not the files they point to                     |
| `skip_symlinks`   | Ignores symlinks                                                 |
| `dirs_only`       | Copies only directories                                          |
| `make_symlinks`   | Creates symlinks of the files instead of copying them            |
| `make_hard_links` | Creates hard links of the files instead of copying them          |

To join more options you can use the bit-wise or operator `|`.

```nest
|#| 'stdfs.nest' = fs

fs.CPO.recursive fs.CPO.make_hard_links | = options
'dir1' 'dir2' options @fs.copy
```

!!!note
    When using `make_symlinks` the source path must be an absolute path unless
    the destination path is in the current directory.
