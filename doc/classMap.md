##Interpreter Classes map
#####Main
```
| -----Lexer
|        |
|        -------------------------------
|        |         |         |         |
|     advance    skipwb   integer  nextToken
|
| -----Parser
|        |
|        -------------------------------
|        |     |       |      |       |
|       eat  factor-->term-->expr   prase
|
| -----Interpreter
|        |
|       ---
|        |
|    Interpreter
| (Post-Order Traversal)
|
```
#####Basic structure
```
    AST

    BinOp(Binary tree,using operator as root)
    Num(Terminal)
```
#####Tool
```
    Exception
    Token
```