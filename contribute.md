# How To Add A Grammer

## Token —— Define the keyword

1. add keyword to `TokenType` in `inc/Token.h`
2. put the string of your keyword and token to a map `reservedKeyword` in `inc/Lexer.h`

## AST —— Let the parser know it

1. define a AST class for the keyword in `inc/AST.h`, and put its name in macro definition at the head of file
2. register a virtual function in class `NodeVisitor` in `inc/NodeVisitor.h`, map name of function to address of function in `visitorMap`
3. define the implement of the virtual function in SemanticAnalyzer and Interpreter

## Parser, SemanticAnalyzer and Interpreter —— Make it executable

write your code in as your mind