# Filesystem library

## Importing

```text
|#| 'stdfs.nest' = fs
```

## Functions

### `[path: Str] @isdir`

Checks if a directory exists at `path`.

### `[path: Str] @mkdir`

Creates a new directory at `path`, succeeds even if the directory exists.

### `[path: Str] @mkdirs`

Creates a directory at `path`, and the parent directories if needed, succeeds
even if the directory already exists.

### `[path: Str] @rmdir`

Removes a directory at `path`, returns the error code of the OS in case of
failure, otherwise returns `0`. If the directory path does not exist, raises an
error.

[Windows error codes](https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-)
[Linux error codes](https://chromium.googlesource.com/chromiumos/docs/+/master/constants/errnos.md)

### `[path: Str] @rmdir_recursive`

Removes a directory at `path` and all of its contents, returns the error code of
the OS in case of failure, otherwise returns `0`. If the directory path does not
exist, raises an error.

[Windows error codes](https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-)
[Linux error codes](https://chromium.googlesource.com/chromiumos/docs/+/master/constants/errnos.md)

### `[path: Str] @isfile`

Checks if a file exists at `path`.

### `[path: Str] @rmfile`

Removes a file at `path`, returns the error code of the OS in case of failure,
otherwise returns `0`. If the path does not exist, raises an error.

[Windows error codes](https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-)
[Linux error codes](https://chromium.googlesource.com/chromiumos/docs/+/master/constants/errnos.md)

### `[from: Str, to: Str, options: Int?] @copy`

Copies a file or a directory from `from` to `to`. `options` must be set with
the `CPO` constant and its members.  
If `options` is `null`, `CPO.none` is used.

### `[old_path: Str, new_path: Str] @rename`

Renames or moves a file or a directory.

### `[path: Str] @list_dir`

Lists the contents of a directory, both files and sub-directories.

### `[path: Str] @list_dir_recursive`

Lists the contents of a directory, and all the sub-directories.

### `[path: Str] @absolute_path`

Returns the absolute path of a file or directory.

### `[path: Str] @canonical_path`

Returns the canonical path of a file or directory. A canonical path is an
absolute path that does not point to a link. This means that any canonical path
is an absolute path but not vice-versa.

### `[path: Str, base: Str] @relative_path`

Returns a relative path to `path` using `base` as the starting point.

```text
|#| 'stdfs.nest' = fs

'/a/c' '/a/b/d' @fs.relative_path --> '../../c'
```

### `[path_1: Str, path_2: Str] @equivalent`

Returns if the two paths point to the same file, note that comparing hard links
and the file itself returns `true`.

### `[path_1: Str, path_2: Str] @join`

Joins two paths by adding, if needed, a slash between them.  
If `path_2` is an absolute path it is returned.  
This function normalizes the slashes after joining: on Windows `/` becomes `\`
and on Linux `\` becomes `/`.

### `[path: Str] @path`

Returns only the folder path where `path` points to a file.

```text
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.path --> 'dir/subdir'
```

### `[path: Str] @filename`

Returns the name of the file `path` points to.

```text
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.filename --> 'file.txt'
```

### `[path: Str] @extension`

Returns the extension of the file pointed to by `path`.

```text
|#| 'stdfs.nest' = fs

'dir/subdir/file.txt' @fs.extension --> '.txt'
```

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

```text
|#| 'stdfs.nest' = fs

fs.CPO.recursive fs.CPO.make_hard_links | = options
'dir1' 'dir2' options @fs.copy
```

> Note: when using `make_symlinks` the source path must be an absolute path
> unless the destination path is in the current directory.
