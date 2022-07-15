# A Very Simple <s>Interpreter</s> Compiler 2

![Github repo size](https://img.shields.io/github/repo-size/zsiothsu/AVSI2)
![Github license](https://img.shields.io/github/license/zsiothsu/AVSI2)
![platform](https://img.shields.io/badge/platform-linux-green)

[大陆简体](README.zh_cn.md) · [臺灣正體](README.zh_tw.md)

## Introduce
### AVSI
AVSI is a project for learning that everyone can use as reference to build your own interpreter.

[Ruslan's Blog](https://ruslanspivak.com/) inspired me. That is a useful and valuable blog for learning.

The grammar of AVSI is like shell which is suitable for learning to build an Interpreter.

### AVSI2
Instead of running on AST, AVSI2 is a compiler with llvm. Code will be compiled to executable file.

## Building
**LLVM** and **llvm-libs** need to be installed in advance. See [The LLVM Compiler Infrastructure](https://llvm.org/). **Clang** is the default compiler used in this project. If you need to change other compilers, please modify the `CC` in `Makefile` and `libavsi/C/Makefile`

Compile and install the project with the following command:

```shell
$ make all -j
$ sudo make install
```

## Usage (using cart)
`cart` is a package manager for use with avsi. Type the following command to create a sl project:

```shell
$ cart new test_project
```

It will create a project folder with a configure file(Cart.toml) and a main file(main.sl).
Then, use `cart build` to build the project:

```
$ cd test_project

$ ls
Cart.toml  main.sl

$ cart build
compile source files
link objs

$ ./build/test_project
Hello World!
```

## Usage (using avsi)
For example, your project folder tree looks like this:

```
.
├── export_mod.sl
├── main.sl
├── mod1
│   ├── __init__.sl
│   └── sub_mod1.sl
├── mod2
│   └── loop.sl
└── mod3
    └── object.sl
```

The file `main.sl` is the root of whole program. Use `avsi` to build the program:

```shell
$ avsi main.sl
```

Note that we only pass the root file to compiler. Dependent files are automatically compiled when an "import" statement is detected in code.

Now we have a folder called `build` including all object files. (You can use option `-o` to change building folder.)

```
.
├── build               # contains all object files
│   ├── export_mod.bc
│   ├── export_mod.o
│   ├── main.o
│   └── ...
├── export_mod.sl
├── main.sl
├── mod1
│   ├── __init__.sl
│   └── sub_mod1.sl
├── mod2
│   └── loop.sl
└── mod3
    └── object.sl

```

Since AVSI does not have the function of automatic linking, you have to use gcc or clang to link the executable file.

```shell
$ objs=($(find . -name "*.o"))
$ gcc $objs -lavsi -no-pie -o ./a.out 
# or clang
$ clang $objs -lavsi -no-pie -o ./a.out 
```

## Grammar
refer to `./example`

## Planned features

- AVSI
  - object methods
  - macro system (probably not)
- libavsi
  - core library
  - file I/O
- cart
  - dynamic library


## License
AVSI2 is licensed under the MIT license
