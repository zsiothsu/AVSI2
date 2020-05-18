<!--
 * @Author: Chipen Hsiao
 * @Date: 2020-05-18
 * @LastEditTime: 2020-05-18 17:45:14
 * @Description: readme
--> 
## A Very Simple Interpreter
---
###构建项目
在工作目录下使用 
```shell
make
```
指令构建项目，生成的二进制文件在 `./build/` 目录下。可执行文件为 `./build/Interpreter`.
若要清除构建，使用
```shell
make clean
```

###使用
```shell
./build/Interpreter targetFile
```
例如要解释器执行 `./test/pro.sl` 使用
```shell
./build/Interpreter ./test/pro.sl
```

###语法
参见 `./test/test.sl`

