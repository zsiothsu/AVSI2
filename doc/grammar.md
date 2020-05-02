<!--
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-17 18:55:07
 * @Description: file content
 -->
#grammar

---
    statement : assignment_statement
              | empty
    assignment_statement : variable ASSIGN expr
    empty :
    expr: term ((PLUS | MINUS) term)*
    term: factor ((MUL | DIV) factor)*
    factor : PLUS factor
           | MINUS factor
           | INTEGER
           | LPAREN expr RPAREN
           | variable
    variable: ID