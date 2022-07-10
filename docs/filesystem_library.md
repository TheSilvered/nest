# Filesystem library (`stdfs.nest` - `fs`)

## Functions

### `[path: Str] @isdir`

Checks if a directory exists at `path`.

### `[path: Str] @mkdir`

Creates a new directory at `path`, succedes even if the directory exists.

### `[path: Str] @mkdirs`

Creates a directory and subdirectories at `path`, succedes even if
the directory/ies exist(s).

### `[path: Str] @rmdir`

Removes a directory at `path`, returns the error code of the OS in case of
failure, otherwise returns `0`. If the directory path does not exist, raises an
error.

### `[path: Str] @rmdir_recursive`

Removes a directory at `path` and all of its contents, returns the error code of
the OS in case of failure, otherwise returns `0`. If the directory path does not
exist, raises an error.

### `[path: Str] @isfile`

Checks if a file exists at `path`.

### `[path: Str] @rmfile`

Removes a file at `path`, returns the error code of the OS in case of failure,
otherwise returns `0`. If the path does not exist, raises an error.

### `[from: Str, to: Str, options: Int] @copy`

Copies a file or a directory from `from` to `to`. `options` must be set with
the `CPO` constant and its members.

### `[old_path: Str, new_path: Str] @rename`

Renames or moves a file or a directory.

### `[path: Str] @list_dir`

Lists the contents of a directory, both files and subdirectories.

### `[path: Str] @list_dir_recursive`

Lists the contents of a directory, and all the subdirectories.

### `[path: Str] @absolute_path`

Returns the absolute path of a file or directory.

### `[path_1: Str, path_2: Str] @equivalent`

Returns if the two paths point to the same file, note that comparing hard links
and the file itself returns `true`.

## Constants

### `CPO`

A map that has the copy options.  
Its members are:

| Name              | Description                                                      |
| ----------------- | ---------------------------------------------------------------- |
| `none`            | Copy a file normally                                             |
| `skip`            | Skips existing files                                             |
| `overwrite`       | Overwrites existing files                                        |
| `update`          | Overwrites existing files if they are older than the file copied |
| `recursive`       | Copies recursively also subdirectories                           |
| `copy_symlinks`   | Copies symlinks, not the files they point to                     |
| `skip_symlinks`   | Ignores symlinks                                                 |
| `dirs_only`       | Copies only directories                                          |
| `make_symlinks`   | Creates symlinks of the files instead of copying them            |
| `make_hard_links` | Creates hard_links of the files instead of copying them          |
