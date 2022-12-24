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

To install Nest on Linux you will need to [compile it from source](#on-linux).

After that, enter the `build` directory and run `update_files.sh`. If everything
succedes, you can run `linux_install.sh` to copy all the files in their locations,
if you need to install the 32-bit version add `x86` as the only argument when
running the script.

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
into from the latest release and extract it into `%APPDATA%/SublimeText/Packages`.

In Sublime Text, in addition to syntax highlighting you can also run a script
with `CTRL + B` and you can comment and un-comment with the default keyboard
shortcuts.

Since Sublime Text does not support input from the user in the output panel,
you have to run the file from a terminal emulator, to do that I suggest
installing a plugin like `Terminus`.

## Building from source

### On Windows

On Windows open `nest.sln` in Visual Studio with the C++ application development
package installed and compile it from there.

To compile the installer you need InnoSetup installed. Then follow these steps

1. Enter `nest_release_folder\`
2. Execute `update_exes_nest.bat` (Nest) or `update_exes.bat` (Python)
3. Compile `installer-script-x__.iss` with InnoSetup
4. The compiled exe should appear in the same directory with the name
   `nest-[VERSION]-x[ARCHITECTURE]-installer.exe`

Keep in mind that if you change the version it has to be changed both in
`src/nest.h` and in `build/installer/common.iss`.

### On Linux

On Linux you will need to install GCC (GNU Compiler Collection) and then run one
of the following commands.

Execute `make all` to compile the main interpreter and the standard library.  
Execute `make` to compile only the main interpreter.  
Execute `make debug` for a fast compilation of the main interpreter.  
Execute `make all-debug` for a fast compilation of the main interpreter and the
standard library.  
Execute `make clean` to remove the previously compiled programs.  
Execute `make x86` to compile the main interpreter for 32-bit systems.  
Execute `make all-x86` to compile the main interpreter and the standard library
for 32-bit systems

When `make debug` or `make all-debug` are ran, the compiled program is inserted
in `unix_debug/`, otherwise the executable is put in `unix_release/`.
