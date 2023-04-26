# nest

![ubuntu_test](https://github.com/TheSilvered/nest/actions/workflows/ubuntu_test.yml/badge.svg)
[![Documentation Status](https://readthedocs.org/projects/nest-docs/badge/?version=latest)](https://nest-docs.readthedocs.io/en/latest/?badge=latest)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/TheSilvered/nest/blob/main/LICENSE)
[![Latest release](https://img.shields.io/github/v/tag/TheSilvered/nest?label=Latest%20release)](https://github.com/TheSilvered/nest/releases)

A programming language with no keywords.

## Features

- Dynamically typed
- Memory safe
- Interpreted
- Has hash maps, vectors and arrays built in
- Can be expanded with C/C++
- Aimed at being consistent within its design
- the standard library includes coroutines, a JSON parser and various other
  helper functions

**This programming language is still in early development and is heavily unstable.
The beta versions are not properly tested and might crash unexpectedly.**

### Planned features

- Regular expressions in the standard library
- A custom graphics and UI standard library built on top of SDL2
- Unicode built-in strings
- Correct encoding/decoding in I/O

## Documentation

The documentation is hosted on ReadTheDocs and can be found [here](https://nest-docs.readthedocs.io/en/latest/).

## Installation

### Windows

To install Nest on Windows download the installer of the latest release and run
it.

You will also need to install Visual C++ Redistributable, an installer for that
can be downloaded at:

| Link                                             | Architecture       |
| ------------------------------------------------ | ------------------ |
| <https://aka.ms/vs/17/release/vc_redist.x64.exe> | 64-bit             |
| <https://aka.ms/vs/17/release/vc_redist.x86.exe> | 32-bit             |

### Linux

To install Nest on Linux you can install the precompiled binaries or
[compile it from source](#on-linux).

To Install it from the binaries download the `.tar.gz` file with the desired
architecture and extract it in the current directory with:

```bash
tar -xzf nest-[VERSION]-[ARCHITECTURE]-linux.tar.gz
```

Now run `./linux_install_[ARCHITECTURE].sh` to copy the binaries to
`/usr/libs/nest` and to `/usr/bin`

## Plugins

### VS Code

To install the VSCode extension download `vs-code-extension-[VERSION].zip`,
extract it into `%USERPROFILE%/.vscode/extensions` and reload the VS Code window.

Currently VS Code supports only syntax highlighting and commenting with keyboard
shortcuts, to run the Nest file you can install Code Runner and add
`"nest": "cd $dir && nest -m $fileName"` inside `"code-runner.executorMap"` in
`settings.json`.

This method does not support input, to do that you can select the option to run
Code Runner in the terminal and remove `-m` from the command.

### Sublime Text

To install the Sublime Text plugin download `sublime-text-plugin-[VERSION].zip`
from the latest release and extract it into `%APPDATA%/SublimeText/Packages`.

In Sublime Text, in addition to syntax highlighting you can also run a script
with `CTRL + B` and you can comment and un-comment with the default keyboard
shortcuts.

Since Sublime Text does not support input from the user in the output panel,
you have to run the file from a terminal emulator, to do that I suggest
installing a plugin like `Terminus`.

## Building from source

### On Windows

On Windows open `build\windows\projects\nest\nest.sln` in Visual Studio with the
C++ application development package installed and compile it from there.

To open one of the standard libraries singularly, you can use the `.sln` file
with the name that has `std` replaced with `nest_`, for example the `stdio.nest`
library can be opened with `nest_io.sln`.

To compile the installer you need InnoSetup installed. Then follow these steps

1. Enter `nest_release_folder\`
2. Run `.\update_exes.bat`, this will use the Nest script by default, execute
   `.\update_exes.bat py` to use Python instead.
3. Compile `installer-script-x__.iss` with InnoSetup
4. The compiled exe should appear in the same directory with the name
   `nest-[VERSION]-x[ARCHITECTURE]-installer.exe`

Keep in mind that if you change the version it has to be updated in
`include\nest.h`, `build\windows\installer\common.iss` and
`build/linux/_make_archives.sh`

### On Linux

On Linux you will need to install GCC (GNU Compiler Collection) and the 32-bit
libraries with this command:

```bash
sudo apt-get -y install build-essential gcc-multilib g++-multilib
```

Now enter `build/linux/makefiles`.

Execute `make all` to compile the main interpreter and the standard library.  
Execute `make` to compile only the main interpreter.  
Execute `make debug` for a fast compilation with debug information of the main
interpreter, this can be used with the GNU Debugger to debug the program.  
Execute `make all-debug` for a fast compilation with debug information of the
main interpreter and the standard library.  
Execute `make clean` to remove the previously compiled programs.  
Execute `make x86` to compile the main interpreter for 32-bit systems.  
Execute `make all-x86` to compile the main interpreter and the standard library
for 32-bit systems.

To compile only one library run `make -f` with the file that has the same name
of the library. When compiling a library you can also add `debug` or `x86` to
compile in debug mode or 32-bit respectively.

When `make debug` or `make all-debug` are ran, the compiled program is inserted
in `build/linux/linux_debug`, otherwise the executable is put in
`build/linux/linux_release/x64` or `build/linux/linux_release/x86`.

To update the `.nest` files of the standard library, run `./_update_files.sh`,
this will use Nest by default, run `./_update_files.sh py` to use Python instead.
The files will appear in `build/linux/linux_libs`.

Finally, to package the binaries run `./_make_archives.sh` and to install them
run `./linux_install_x64.sh` or `./linux_install_x86.sh` as specified in the
[installation](#linux) paragraph.

You can also run `./_install_debug.sh` to install the debug binaries.
