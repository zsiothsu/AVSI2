# A Very Simple <s>Interpreter</s> Compiler 2

[English](README.md) · [大陆简体](README.zh_cn.md)

## 介紹
### AVSI
AVSI 是一個我用來學習直譯器的項目。看到這個項目的你也可以參考這個項目，嘗試著做一個直譯器或編譯器

[Ruslan 的博客](https://ruslanspivak.com/)啟發了我。這是一個對學習如何編寫直譯器很有幫助的博客。

AVSI的語法類似shell。因為語法較爲簡單，所以適合拿來練習做編譯器。

### AVSI2
AVSI2 不是在 AST 上運行的，而是一個以 llvm 為後端的編譯器。程式碼將被編譯為執行檔。

## 建構
**LLVM** 和 **llvm-libs** 需要提前安裝。參閱 [The LLVM Compiler Infrastructure](https://llvm.org/)。 **Clang** 是此項目中使用的默認編譯器。如需更換其他編譯器，請修改`Makefile`和`libavsi/C/Makefile`中的`CC`

使用以下命令編譯並安裝項目：

```shell
$ make all -j
$ sudo make install
```

## 用法
例如一個項目檔案樹如下所示：

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

檔案 `main.sl` 是整個程式的根。使用 `avsi` 建構程式：

```shell
$ avsi main.sl
```

注意，我們只將根檔案傳遞給編譯器。當在程式碼中檢測到「import」語句時，會自動編譯相關檔案。

現在我們有一個名為 `build` 的檔案夾，其中包含所有目標檔案。 （您可以使用選項 `-o` 更改建構檔案夾。）

```
.
├── build # 包含所有目標檔案
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

由於AVSI沒有自動連結的功能，所以必須使用gcc或clang來連結執行檔。

```shell
$ objs=($(find . -name "*.o"))
$ gcc $objs -lavsi -no-pie -o ./a.out 
# 或者使用 clang
$ clang $objs -lavsi -no-pie -o ./a.out 
```

## 語法
參考`./example`

## 許可證
AVSI2 使用 MIT 許可證