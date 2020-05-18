<!--
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
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