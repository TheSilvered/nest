# Creating your own C library

## Installation of the source

First things first you need to clone the latest stable release of Nest by
executing:

```text
git clone -b beta-0.12.1 https://github.com/TheSilvered/nest.git
cd nest
```

Then you can compile it from source as specified in the
[installation](../installation.md) documentation.

Here is a little explanation of what all the folders contain:

```text
nest
├───build # various files to compile Nest and the installers
├───docs # the documentation
├───examples # some examples of Nest code
├───include # the header files for the core interpreter
├───libs # the standard libraries
├───plugins # the plugins for SublimeText and VSCode
├───src # the source of the core interpreter
├───tests # the tests for the libraries and the interpreter
└───tools # tools for developing Nest
```


