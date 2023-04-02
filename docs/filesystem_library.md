# Filesystem library

## Importing

```nest
|#| 'stdfs.nest' = fs
```

## Functions

### `@is_dir`

**Synopsis**:

`[path: Str] @is_dir -> Bool`

**Description**:

Checks if a directory exists at `path`.

**Arguments**:

- `path`: the path of the directory to check

---

### `@make_dir`

**Synopsis**:

`[path: Str] @make_dir -> null`

**Description**:

Creates a new directory at `path`, succeeds even if the directory exists. An
error is thrown if any parent directory does not exist or a system error occurs.

**Arguments**:

- `path`: the path of the directory to create

---

### `@make_dirs`

**Synopsis**:

`[path: Str] @make_dirs -> null`

**Description**:

Creates a directory at `path`, and the parent directories if needed, succeeds
even if the directory already exists. An error is thrown only if a system error
occurs.

**Arguments**:

- `path`: the path of the directories to create

---

### `@remove_dir`

**Synopsis**:

`[path: Str] @remove_dir -> null`

**Description**:

Removes a directory at `path` which must be empty. An error is thrown if the
directory contains any sub directories or files, the path does not point to a
valid directory or if a system error occurs.

**Arguments**:

- `path`: the path of the directory to delete

---

### `@remove_dirs`

**Synopsis**:

`[path: Str] @remove_dirs -> null`

**Description**:

Removes a directory at `path` and all of its contents, throws an error if the
directory does not exist or a system error occurs.

**Arguments**:

- `path`: the path of the directory to delete

---

### `@is_file`

**Synopsis**:

`[path: Str] @is_file -> Bool`

**Description**:

Checks if a file exists at `path`.

**Arguments**:

- `path`: the path of the file to check

---

### `@remove_file`

**Synopsis**:

`[path: Str] @remove_file -> null`

**Description**:

Removes a file at `path`, throws an error if the file does not exist or a system
error occurs.

**Arguments**:

- `path`: the path of the file to delete

---

### `@copy`

**Synopsis**:

`[from: Str, to: Str, options: Int?] @copy -> null`

**Description**:

Copies a file or a directory from `from` to `to`. `options` must be set with
the [`CPO`](#cpo) constant and its members.  
If `options` is `null`, `CPO.none` is used.  
An error is thrown if either the source or the destination path does not exits
or if a system error occurs.

**Arguments**:

- `from`: the path of the element to copy
- `to`: the path the element should be copied as
- `options`: the options that modify the default behaviour of copy

---

### `@rename`

**Synopsis**:

`[old_path: Str, new_path: Str] @rename -> null`

**Description**:

Renames or moves a file or a directory. Throws an error if `old_path` does not
exist or if a system error occurs.

**Arguments**:

- `old_path`: the old path of the element
- `new_path`: the path the element should be moved to

---

### `@list_dir`

**Synopsis**:

`[path: Str] @list_dir -> Vector`

**Description**:

Lists the contents of a directory, both files and sub-directories but not the
contents of the latter. Throws an error if `path` does not point to a valid
directory or a system error occurs.

**Arguments**:

- `path`: the directory of which the contents are listed

---

### `@list_dirs`

**Synopsis**:

`[path: Str] @list_dirs -> Vector`

**Description**:

Similar to [`list_dir`](#list_dir) but also lists the contents of the
sub directories. Throws an error if `path` does not point to a valid directory
or a system error occurs.

**Arguments**:

- `path`: the directory of which the contents are listed

---

### `@absolute_path`

**Synopsis**:

`[path: Str] @absolute_path -> Str`

**Description**:

Returns the absolute path of a file or directory.

**Arguments**:

- `path`: the relative path to expand

---

### `@canonical_path`

**Synopsis**:

`[path: Str] @canonical_path -> Str`

**Description**:

Returns the canonical path of a file or directory. A canonical path is an
absolute path that does not point to a link. This means that any canonical path
is an absolute path but not vice-versa.

**Arguments**:

- `path`: the relative path to expand

---

### `@relative_path`

**Synopsis**:

`[path: Str, base: Str] @relative_path -> Str`

**Description**:

Returns a relative path to `path` using `base` as the starting point.

**Arguments**:

- `path`: the directory to reach
- `base`: the starting directory

**Example**:

```nest
|#| 'stdfs.nest' = fs

'/a/c' '/a/b/d' @fs.relative_path --> '../../c'
```

---

### `@equivalent`

**Synopsis**:

`[path_1: Str, path_2: Str] @equivalent -> Bool`

**Description**:

Returns true if the two paths point to the same file and false otherwise. This
returns true even when the two paths are different but are hard links to the
same file.

**Arguments**:

- `path_1`: the path of the first file
- `path_2`: the path of the second file

---

### `@join`

**Synopsis**:

`[path_1: Str, path_2: Str] @join -> Str`

**Description**:

Joins two paths by adding, if needed, a slash between them.  
If `path_2` is an absolute path it is returned without any modifications.  
This function normalizes the slashes after joining: on Windows `/` becomes `\`
and on Linux `\` becomes `/`.

**Arguments**:

- `path_1`: the first path
- `path_2`: the path to append to `path_1`

---

### `@parent_path`

**Synopsis**:

`[path: Str] @parent_path -> Str`

**Description**:

Returns the path of the directory where the element is contained.

**Arguments**:

- `path`: the path to a file or a directory

**Example**:

```nest
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.parent_path --> 'dir/subdir'
'dir/subdir/subdir2'  @fs.parent_path --> 'dir/subdir'
'dir/subdir/subdir2/' @fs.parent_path --> 'dir/subdir/subdir2'
```

---

### `@filename`

**Synopsis**:

`[path: Str] @filename -> Str`

**Description**:

Returns the name of the file `path` points to.

**Arguments**:

- `path`: the path of a file

**Example**:

```nest
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.filename --> 'file.txt'
'dir/subdir/subdir2'  @fs.filename --> 'subdir2'
'dir/subdir/subdir2/' @fs.filename --> ''
```

---

### `@extension`

**Synopsis**:

`[path: Str] @extension -> Str`

**Description**:

Returns the extension of the file pointed to by `path`.

**Arguments**:

- `path`: the path of a file

```nest
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.extension --> '.txt'
'dir/subdir/file.tar.gz' @fs.extension --> '.gz'
'dir/subdir/file' @fs.extension --> ''
'dir/subdir/subdir2' @fs.extension --> ''
'dir/subdir/subdir2/' @fs.extension --> ''
```

---

## Constants

### `CPO`

A map that has the copy options.  
Its members are:

| Name              | Description                                                      |
| ----------------- | ---------------------------------------------------------------- |
| `none`            | Normal copy, follows symlinks and skips subdirectories           |
| `skip`            | Skips existing files                                             |
| `overwrite`       | Overwrites existing files                                        |
| `update`          | Overwrites existing files if they are older than the file copied |
| `recursive`       | Copies recursively also sub-directories                          |
| `copy_symlinks`   | Copies symlinks, not the files they point to                     |
| `skip_symlinks`   | Ignores symlinks                                                 |
| `dirs_only`       | Copies only directories                                          |
| `make_symlinks`   | Creates symlinks of the files instead of copying them            |
| `make_hard_links` | Creates hard_links of the files instead of copying them          |

To join more options you can use the bit-wise or operator `|`.

```nest
|#| 'stdfs.nest' = fs

fs.CPO.recursive fs.CPO.make_hard_links | = options
'dir1' 'dir2' options @fs.copy
```

!!!note
    when using `make_symlinks` the source path must be an absolute path unless
    the destination path is in the current directory.
