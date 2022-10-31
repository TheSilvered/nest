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

After using the installer you can edit the `PATH` environment variable relative
to the account (*not* relative to the system) by adding
`%LOCALAPPDATA%\Programs\nest`. This is to add Nest as a command when using the
Command Prompt or Windows PowerShell.

## Building from source

### Windows

On windows open `nest.sln` in Visual Studio with the C++ application develompent
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

**C/C++ libraries do not work on linux yet.**

Execute `make all` to compile the main interpreter and all the libraries.  
Execute `make` to compile only the main iterpreter.
Execute `make debug` for a fast compilation of the main interpreter.
Execute `make all-debug` for a fast compilation of the main interpreter and all
the libraries.
Execute `make clean` to remove the previously compiled programs.

When `make debug` or `make all-debug` are ran, the compiled program is inserted
in `unix_debug/`, otherwise the executable is put in `unix_release/`
