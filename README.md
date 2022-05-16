# A Very Simple <s>Interpreter</s> Compiler 2

## Introduce
### AVSI
AVSI is a project for learning that everyone can use as reference to build your own interpreter.

[Ruslan's Blog](https://ruslanspivak.com/) inspired me. That is a useful and valuable blog for learning.

The grammar of AVSI is like shell which is suitable for learning to build an Interpreter.

### AVSI2
Instead of running on AST, AVSI2 is a compiler with llvm. Code will be compiled to executable file.

## Usage
Use `avsi [options] file` , it will generate object file `a.o`.
```shell
$ avsi yourcode.sl
```
About options, see `avsi --help`

use `clang` to link obj file generated and `libavsi.a`
```shell
$ clang yourcode.o -L [Path to libavsi.a] -lavsi
```

run executable file
```asm
$ ./a.out
```

## Grammar
refer to `./example`

## License
AVSI2 is licensed under the MIT license
