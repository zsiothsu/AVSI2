# A Very Simple Interpreter

## Introduce
AVSI is a project for learning that everyone can use as reference to build your own interpreter.

[Ruslan's Blog](https://ruslanspivak.com/) inspired me. That is a useful and valuable blog for learning.

The grammar of AVSI is like shell which is suitable for learning to build an Interpreter.


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
./build/Interpreter ./example/pro.sl --scope
```

## Grammar
refer to `./example`

## What's working

|  **function**   | **Status**  |
| :-------------: | :---------: |
|       I/O       |   partial   |
|   assignment    |  supported  |
|   expression    |  supported  |
|    function     |  supported  |
|  if statement   | coming soon |
|  for statement  | unsupported |
| while statement | unsupported |
|     boolean     |  supported  |
|     number      |  supported  |
|     string      | unsupported |

## License
AVSI is licensed under the MIT license
