<!--
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-18 16:43:19
 * @Description: file content
 -->
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
```mermaid
classDiagram
NodeVisitor --> Interpreter:泛化
Interpreter --|> Parser:依赖
Interpreter:Interpret() int
Parser --|> Lexer:依赖
Parser --|> AST:依赖
Parser --|> Exception:依赖
Parser --|> Token:依赖
Parser: Lexer* lexer
Parser: Token currentToken
Parser: eat() void
Parser: parse() AST*
Lexer:string line
Lexer:int cur
Lexer: currentChar
Lexer:getNextToken() Token
Interpreter --|> Data:依赖

Token: int valueInt
Token: char valueChar
Token: CharType type
Token: getType() CharType
Token: getValue() int
Token: __str() string

AST --> BinOP:泛化
AST --> Num:泛化
BinOP: AST* left
BinOP: AST* right
Num:Token token
Num:int value

Data --|> DataType:依赖
Data: DataType type
Data: int valueInt
DataType:enum

Exception --> SyntaxException:泛化
Exception --> MathException:泛化
Exception:what()
```

<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>


```mermaid
classDiagram
外面的电脑 --> 服务器 :直接访问
服务器-->你家的NAS :内网穿透
你家的NAS --> 服务器
服务器-->外面的电脑
服务器-->外网的服务器 :直接访问
外网的服务器 --> 服务器

```