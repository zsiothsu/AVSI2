<!--
 * @Author: Chipen Hsiao
 * @Date: 2020-05-18
 * @LastEditTime: 2020-05-27 15:32:33
 * @Description: readme
--> 
# A Very Simple Interpreter

## Introduce
AVSI is a project for learning that everyone can use as reference to build your own interpreter.

[Ruslan's Blog](https://ruslanspivak.com/) inspired me.That is a useful and valuable blog for learning.

The grammar of AVSI is like shell which is suitable for learning to build an Interpreter.

## Prerequisite
**[gflags](https://github.com/gflags/gflags)** is a library for handling command line parameters. It must be installed before using AVSI.

##### Debian/Ubuntu Linux
```shell
sudo apt-get install libgflags-dev
```
##### Arch Linux
```shell
sudo pacman -S gflags
```
##### macOS
```shell
brew install gflags
```
##### for other linux
Using these commmand to install gflags
```shell
git clone https://github.com/gflags/gflags.git
cd gflags-2.2.2 #it depends on the name of gflags folder
mkdir build && cd build
cmake ..
make
make install
```


## Build Instructions
On Unix and Linux, using the command `make` to build project.
[*For windows*],Unix shell tools([msys](http://www.mingw.org/wiki/MSYS), [Cygwin](http://www.cygwin.com/), [GNUwin32](http://gnuwin32.sourceforge.net/)) are needed

Executable file is `./build/Interpreter`

## Usage
```
Interpreter file [--scope] [--callStack]
```

example:
```
./build/Interpreter ./test/pro.sl --scope
```

## Grammar
refer to `./doc/grammar.sl`

## License
AVSI is licensed under the MIT license
