# Installation

## Windows

### Installer

To install Nest on Windows you will first need to install Visual C++
Redistributable that can be downloaded here:

| Link                                             | Architecture       |
| ------------------------------------------------ | ------------------ |
| <https://aka.ms/vs/17/release/vc_redist.x64.exe> | 64-bit             |
| <https://aka.ms/vs/17/release/vc_redist.x86.exe> | 32-bit             |

After installing it you can download the installer from the desired release and
run it. It is recommended to add Nest to the PATH environment variable to use
it from the terminal.

Once installed you can check that it is working by running this command:

```text
PS C:\> nest -V
Using Nest version: beta-0.16.0 x64
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

## Linux

### Compile from source on Linux

Compiling on linux requires `gcc` or `clang` as the C/C++ compiler.

To compile on linux go to `build/linux`, from here you can perform one of the
following commands:

```sh
make install  # compile and install nest and the standard library
make uninstall  # remove a previous installation of nest
make all  # only compile nest and the standard library
make help  # to see all other make options
```

If you are using clang use the following commands to compile Nest:

```sh
make install CC=clang
make all CC=clang
```

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
with `CTRL + B` and you can add and remove comments with the default keyboard
shortcuts.

Since Sublime Text does not support input from the user in the output panel,
you have to run the file from a terminal emulator, to do that I suggest
installing a plugin like `Terminus`.
