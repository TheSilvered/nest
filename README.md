# nest
A programming language with no keywords.

## Features
- Dynamically typed
- Memory safe
- Interpreted
- Function-oriented
- Has named variables
- Has hash maps, vectors and arrays built in
- Can be expanded with C/C++

## Quick start

Nest currently only works on Windows and can be installed via the `.exe`
installer.

### VS Code

To install the VS Code plugin copy `plugins/vs_code/nest-lang` into
`%USERPROFILE%/.vscode/extensions` and reload the VS Code window.

Currently VS Code supports only syntax highlighting, to run the Nest file you
can install Code Runner and add `"nest": "cd $dir && nest -m $fileName"` inside
`"code-runner.executorMap"` in `settings.json`.

This method does not support input, to do that you still have to run the file
from the console.

### Sublime Text

To install the SublimeText plugin copy `plugins/subilme_text/Nest` into
`%APPDATA%/SublimeText/Packages`.

In Sublime Text, in addition to syntax highlighting you can also run a script
with `CTRL + B` and you can comment and un-comment with the default keyboard
shortcuts.

Like VS Code, Sublime Text does not support input from the user in the output
panel, to do that you can install a plugin like `Terminus`.

**When downloading the plugins I suggest downloading them with the code of the
most recent commit rather than from the source code of the latest release.**

## Building from source

### Windows

On windows open `nest.sln` in Visual Studio with the C++ application development
package installed and compile it from there.

To compile the installer you need InnoSetup installed. Then follow these steps
1. Enter `nest_release_folder\`
2. Execute `update_exes_nest.bat` (Nest) or `update_exes.bat` (Python)
3. Compile `installer-script-x__.iss` with InnoSetup
4. The compiled exe should appear in the same directory with the name
   `nest-[VERSION]-x[ARCHITECTURE]-installer.exe`

Keep in mind that if you change the version it has to be changed both in
`nest.h` and in the installers' scripts.

### Linux

**C/C++ libraries do not work on Linux yet.**

Execute `make all` to compile the main interpreter and all the libraries.  
Execute `make` to compile only the main interpreter.
Execute `make debug` for a fast compilation of the main interpreter.
Execute `make all-debug` for a fast compilation of the main interpreter and all
the libraries.
Execute `make clean` to remove the previously compiled programs.

When `make debug` or `make all-debug` are ran, the compiled program is inserted
in `unix_debug/`, otherwise the executable is put in `unix_release/`
