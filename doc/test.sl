###
 # @Author: your name
 # @Date: 1970-01-01 08:00:00
 # @LastEditTime: 2020-05-11 15:38:26
 # @Description: file content
 ###

#basic expression
1 * 2 + 3.0 - 4.0 / 5.0 / (6.0 * 1e2)
a = 1
b = a + 1

#if
if [a>0 && b>0] then
    some code
elif [a<0 && b<0] then
    some code
else
    some code
fi

#for
for var in range
do
    somcode
done

#while
while condition
do
    somecode
done

#function
function name (a,b,c,d)
{
    some code..
    return val(optional)
}

#print
print "{1} is a {2}" ("this","very simple language")


#--------------------------------------------#
#                   token                    #
#--------------------------------------------#
---------------------------------------------
Token                       Value
---------------------------------------------
END                         EOF(-1)
NONE                        0
    -------------------------------------
INT                         from -2,147,483,648 to 2,147,483,647
FLT                         from 2.3E-308 to 1.7E+308
    -------------------------------------
ADD                         '+'
DEC                         '-'
MUL                         '*'
DIV                         '/'
    -------------------------------------
LPAREN                      '('
RPAREN                      ')'
ASSIGN                      '='
    -------------------------------------
VAR                         begin with alpha or '_'
FUNNAME                     begin with alpha or '_'


#--------------------------------------------#
#                  grammer                   #
#--------------------------------------------#
---------------------------------------------
'a', 'b', 'c'           : CHAR
1, 2, 3                 : INTEGER
1.0, 3e-2               : FLOAT
"this is a string"      : STRING
True False              : BOOL
('a',1,1.0,"string")    : ARRAY
---------------------------------------------
expr: term (PLUS|DEC term)*
term: factor (MUL|DIV factor)*
factor: : PLUS factor
        | MINUS factor
        | INTEGER
        | LPAREN expr RPAREN
        | variable
        | function
---------------------------------------------
assignment_statement: VAR ASSIGN expr
VAR: ID
---------------------------------------------
program: statement_list
statement_list: statement semi (statement_list)*
statement: assignment_statement
         | echo
         | funtion
         | return
---------------------------------------------
funtion: FUNCTION NAME ARRAY compound_statement
compound_statement: L{ (statement_list)* R}
return: expr
---------------------------------------------
echo: ECHO STR ARRAY
---------------------------------------------


