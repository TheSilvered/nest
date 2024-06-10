# nest

![ubuntu_test](https://github.com/TheSilvered/nest/actions/workflows/ubuntu_test.yml/badge.svg)
[![Documentation Status](https://readthedocs.org/projects/nest-docs/badge/?version=latest)](https://nest-docs.readthedocs.io/en/latest/?badge=latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/TheSilvered/nest/blob/main/LICENSE)
[![Latest release](https://img.shields.io/github/v/tag/TheSilvered/nest?label=Latest%20release)](https://github.com/TheSilvered/nest/releases)

A programming language with no keywords.

**This programming language is still in development and can be unstable. Beta
versions are not properly tested and might crash unexpectedly.**

## Features

- Dynamically typed
- Memory safe
- Interpreted
- Has hash maps, vectors and arrays built in
- Can be expanded with C/C++
- Aimed at being consistent within its design
- the standard library includes coroutines, a JSON parser and various other
  helper functions

### Nest 0.15.0 planned features

- [x] Improve `libnest` usage:
  - [x] Separate execution and interpreter states
  - [x] Initialize everything with one function
  - [x] Quit everything with one function
  - [x] Execute a program in one function (given the path of the file)
- [x] Rewrite nodes, parser, compiler and optimizer
- [x] Rename `co.pause` to `co.yield`
- [x] Make the return value of `co.yield` the arguments passed to `co.call`
  when it is used to restart it
- [x] Change `co.call` to accept less arguments than the function requires
- [x] Change `err.try` to accept less arguments than the function requires
- [x] Add `is_space`, `lremove` and `rremove` to `stdsutil.nest`
- [x] Add `batch` to `stditutil.nest`
- [x] Add `enum` to `stdsequtil.nest`
- [x] Change `generator` in `stdco.nest` to allow for different arguments of
  the function
- [x] Change `relative_path` in `stdfs.nest` to use the current working
  directory when no `base` is given
- [x] Add `get_capacity` to `stdsys.nest`
- [x] Improve option handling in `stdjson.nest`
- [ ] Better testing with C
- [ ] Finish implementing `Nst_fmt` and add a wrapper to `stdsutil.nest`
- [ ] Rename `split` to `lsplit` and add `rsplit` to `stdsutil.nest`
- [ ] Remove `reversed` from `stditutil.nest` and add `reverse` and `reverse_i`
  to `stdsequtil.nest`
- [ ] Remove `_cwd_` and rename `_get_cwd` and `_set_cwd` to `get_cwd` and
  `set_cwd` in `stdsys.nest`

### Future planned features

- Regular expressions in the standard library [^1]
- A custom graphics and UI standard library built on top of SDL2 [^2]
- Closures
- Multithreading

[^1]: will probably be a port of PCRE

[^2]: currently in the workings and undocumented

## Documentation

The documentation is hosted on ReadTheDocs and can be found
[here](https://nest-docs.readthedocs.io/).

## Installation

### Windows Installer

To install Nest on Windows you will first need to install Visual C++
Redistributable that can be downloaded here:

| Link                                             | Architecture       |
| ------------------------------------------------ | ------------------ |
| <https://aka.ms/vs/17/release/vc_redist.x64.exe> | 64-bit             |
| <https://aka.ms/vs/17/release/vc_redist.x86.exe> | 32-bit             |

After insalling it you can download the installer from the desired release and
run it. It is recommended to add Nest to the PATH environment variable to use
it from the terminal.

Once installed you can check that it is working by running this command:

```text
PS C:\Users\user> nest -V
Using Nest version: beta-0.15.0 x64
```

### Compile from source on Windows

To compile Nest from source you will need to install Visual Studio 17 or newer
with the Desktop development with C++ package.

Once installed you can clone the repository and open the solution located in
`nest\build\windows\projects\nest\nest.sln` and compile it from there.

To create the installer you need to install InnoSetup, then navigate to
`nest\build\windows` and run `update_exes.bat`.

For this step you need to have either Nest already installed or Python 3 on
your machine, in case you want to use Python run `update_exes.bat py`.

Once done you can navigate to `nest\build\windows\installer` and compile
`installer-script-x64.iss` and `installer-script-x86.iss`.

### Uninstalling on Windows

To uninstall Nest on Windows you can do so from the control panel.

### Linux archive

To Install Nest from the precompiled binaries archive download the `.tar.gz`
file with the desired architecture and extract it in the current directory with:

```text
$ tar -xzf nest-[VERSION]-[ARCHITECTURE]-linux.tar.gz
```

Now run `./linux_install_[ARCHITECTURE].sh` to copy the binaries to
`/usr/libs/nest` and to `/usr/bin` and install the necessary libraries.

### Compile from source on Linux

To compile Nest from source on Linux first clone the repository and enter in the
directory. From here run `sudo bash configure.sh` to install the necessary
libraries.

**Note**: if you are not using `apt` or `dnf` this file will not work.

Once you have installed the libraries you can enter `nest/build/linux/makefiles/`
and run `make help` to see what to compile. In general you will want to run
`make install` or `make install-x86`.

### Uninstalling on linux

To uninstall Nest on linux, copy
[`nest/build/linux/uninstall.sh`](https://github.com/TheSilvered/nest/blob/main/build/linux/uninstall.sh)
and execute it.

## Plugins

### VS Code

To install the VSCode extension download `vs-code-extension-[VERSION].zip`,
extract it into `%USERPROFILE%\.vscode\extensions` on Windows or into
`~/.vscode/extensions` on Linux and reload the VS Code window.

Currently VS Code supports only syntax highlighting and commenting with keyboard
shortcuts, to run the Nest file you can install Code Runner and add
`"nest": "cd $dir && nest $fileName"` inside `"code-runner.executorMap"` in
`settings.json`.

This method does not support input, to do that you can select the option to run
Code Runner in the terminal.

### Sublime Text

To install the Sublime Text plugin download `sublime-text-plugin-[VERSION].zip`
from the latest release and extract it into `%APPDATA%/SublimeText/Packages`
on Windows or into `~/.config/sublime-text/Packages` on Linux.

In Sublime Text, in addition to syntax highlighting you can also run a script
with `CTRL + B` and you can comment and un-comment with the default keyboard
shortcuts.

Since Sublime Text does not support input from the user in the output panel,
you have to run the file from a terminal emulator, to do that I suggest
installing a plugin like `Terminus`.
