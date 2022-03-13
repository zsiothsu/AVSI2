# A Very Simple Interpreter 2

## Introduce
AVSI is a project for learning that everyone can use as reference to build your own interpreter.

[Ruslan's Blog](https://ruslanspivak.com/) inspired me. That is a useful and valuable blog for learning.

The grammar of AVSI is like shell which is suitable for learning to build an Interpreter.

Instead of running on AST, AVSI2 runs with llvm.

## Usage
use `avsi [source file]` , it will generate `a.ll` and `a.o`
```shell
$ avsi yourcode.sl
```

use `clang` to link obj file generated and `libavsi.a`
```shell
$ clang a.o -L [Path to libavsi.a] -lavsi
```

run executable file
```asm
$ ./a.out
```

## Grammar
refer to `./example`

## License
AVSI2 is licensed under the MIT license
