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

## Building

### Windows

On windows open `nest.sln` in Visual Studio with the C++ application develompent
package installed and compile it from there.

To compile the installer you need InnoSetup installed. Then follow these steps
1. Enter `nest_release_folder\`
2. Execute `update_exes_nest.bat` (Nest) or `update_exes.bat` (Python)
3. Compile `installer-script-x__.iss` with InnoSetup
4. The compiled exe should appear in the same directory with the name
   `nest-[VERSION]-x[ARCHITECTURE]-installer.exe`

### Linux

Execute `make all` to compile the main interpreter and all the libraries.  
Execute `make` to compile only the main iterpreter.
Execute `make debug` for a fast compilation of the main interpreter.
Execute `make libs` for a fast compilation of the libraries.
Execute `make clean` to remove the previously compiled programs.
