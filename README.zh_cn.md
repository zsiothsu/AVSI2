# A Very Simple <s>Interpreter</s> Compiler 2

[English](README.md) · [臺灣正體](README.zh_tw.md)

## 介绍
### AVSI
AVSI 是一个我用来学习解释器的项目。看到这个项目的你也可以参考这个项目，尝试着做一个解释器或编译器

[Ruslan 的博客](https://ruslanspivak.com/)启发了我。这是一个对学习如何编写解释器很有帮助的博客。

AVSI的语法类似shell。因为语法比较简单，所以适合拿来练习做编译器。

### AVSI2
AVSI2 不是在 AST 上运行的，而是一个以 llvm 为后端的编译器。代码将被编译为可执行文件。

## 构建
**LLVM** 和 **llvm-libs** 需要提前安装。参阅 [The LLVM Compiler Infrastructure](https://llvm.org/)。 **Clang** 是此项目中使用的默认编译器。如需更换其他编译器，请修改`Makefile`和`libavsi/C/Makefile`中的`CC`

使用以下命令编译并安装项目：

```shell
$ make all -j
$ sudo make install
```

## 用法
例如一个项目文件树如下所示：

```
.
├── export_mod.sl
├── main.sl
├── mod1
│ ├── __init__.sl
│ └── sub_mod1.sl
├── mod2
│ └── loop.sl
└── mod3
    └── object.sl
```

文件 `main.sl` 是整个程序的根文件。使用 `avsi` 构建程序：

```shell
$ avsi main.sl
```

注意，我们只将根文件传递给编译器。当在代码中检测到“import”语句时，会自动编译相关文件。

现在我们有一个名为 `build` 的文件夹，其中包含所有目标文件。 （您可以使用选项 `-o` 更改构建文件夹。）

```
.
├── build # 包含所有目标文件
│ ├── export_mod.bc
│ ├── export_mod.o
│ ├── main.o
│ └── ...
├── export_mod.sl
├── main.sl
├── mod1
│ ├── __init__.sl
│ └── sub_mod1.sl
├── mod2
│ └── loop.sl
└── mod3
    └── object.sl

```

由于AVSI没有自动链接的功能，所以必须使用gcc来链接可执行文件。

```shell
$ objs=($(find .-name "*.o"))
$ gcc $objs -lavsi -o ./a.out
```

## 语法
参考`./example`

## 许可证
AVSI2 使用 MIT 许可证