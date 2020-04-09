/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-04-09 20:49:28
 * @Description: some methods for Interpreter class
 */

#include "Interpreter.h"

namespace INTERPRETER
{
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    /**
     * @description:    default constructor
     * @param:          None
     * @return:         None
     */
    Interpreter::Interpreter()
    {
    }

    /**
     * @description:    default destructor
     * @param:          None
     * @return:         None
     */
    Interpreter::~Interpreter()
    {
    }

    /**
     * @description:    to initialize a Interpreter.
     * @param:          {string}line:command to be executed
     * @return:         None
     */
    Interpreter::Interpreter(string line)
    {
        this->line = line;
        this->cur = 0;
        this->currentChar = this->line[0];
        this->currentToken = Token::empty();
    }

    /*******************************************************
     *                  lexical analyzer                   *
     *******************************************************/
    /**
     * @description:    advance the "cur" pointer and set the "currentChar" variable
     * @param:          None
     * @return:         None
     */
    void Interpreter::advance()
    {
        this->cur += 1;
        if(this->cur < this->line.length())
        {
            this->currentChar=this->line[this->cur];
        }
        else
        {
            this->currentChar = EOF;
        }
    }

    /**
     * @description:    a lexical analyzer(scanner,tokenizer)
     *                  breaking a sentence apart into tokens.
     * @param:          None
     * @return:         a token for parser
     */
    Token Interpreter::getNextToken()
    {
        while(this->currentChar != EOF)
        {
            if(this->currentChar == ' ')
            {
                skipWhiteSpace();
                continue;
            }
            if(isdigit(this->currentChar))
            {
                return Token(INT,integer());
            }
            if(this->currentChar == '+')
            {
                advance();
                return Token(ADD,'+');
            }
            if(this->currentChar == '-')
            {
                advance();
                return Token(DEC,'-');
            }
            if(this->currentChar == '*')
            {
                advance();
                return Token(MUL,'*');
            }
            if(this->currentChar == '/')
            {
                advance();
                return Token(DIV,'/');
            }

            return Token::empty();
        }
        return Token(END,EOF);
    }

    /**
     * @description:    extract a (multidigit) integer from the sentence
     * @param:          None
     * @return:         an integer
     */
    int Interpreter::integer()
    {
        int num = 0;
        while(isdigit(this->currentChar))
        {
            num = num * 10 + this->currentChar - '0';
            advance();
        }
        return num;
    }

    /**
     * @description:    skip white space
     * @param:          None
     * @return:         None
     */
    void Interpreter::skipWhiteSpace()
    {
        while((this->currentChar != EOF) && (this->currentChar == ' '))
        {
            advance();
        }
    }
    /*******************************************************
     *                parser / interpreter                 *
     *******************************************************/
    /**
     * @description:    compare the current token type with the passed token
     *                  type and if they match then "eat" the current token
     *                  and assign the next token to the self.current_token,
     *                  otherwise raise an exception.
     * @param:          type: token type to be compared with current token
     * @return:         None
     * @throw:          SyntaxException
     */
    void Interpreter::eat(Type type)
    {
        if(type == currentToken.getType())
        {
            this->currentToken = getNextToken();
        }
        else
        {
            throw ExceptionFactory("SyntaxException","invalid syntax");
        }
    }

    /**
     * @description:    arithmetic expression parser / interpreter.
     * @param:          None
     * @return:         result of arithmetic expression
     * @throw:          SyntaxException
     * @grammar:        expr: term ((ADD | DEC) term)*
     *                  term: integer;
     */
    int Interpreter::expr()
    {
        try
        {
            this->currentToken = getNextToken();
            int res = term();
            while(this->currentToken.getType() == ADD || \
                  this->currentToken.getType() == DEC)
            {
                Token opt = this->currentToken;
                if(opt.getValue() == '+')
                {
                    eat(ADD);
                    res = res + term();
                }
                else if(opt.getValue() == '-')
                {
                    eat(DEC);
                    res = res - term();
                }
                else throw ExceptionFactory("SyntaxException","invalid syntax");;
            }
            return res; 
        }
        catch(Exception& e)
        {
            throw e;
        }
    }

    /**
     * @description:    return an integer factor
     * @param:          none
     * @return:         integer
     * @grammar:        integer
     */
    int Interpreter::factor()
    {
        Token token = this->currentToken;
        eat(INT);
        return token.getValue();
    }

    /**
     * @description:    return MUL / DIV result 
     * @param:          None
     * @return:         integer
     * @grammar:        term: factor ((MUL | DIV) factor)*
     *                  factor: integer
     */
    int Interpreter::term()
    {
        int res = factor();
        while(this->currentToken.getType() == MUL || \
              this->currentToken.getType() == DIV)
        {
            Token opt = this->currentToken;
            if(opt.getValue() == '*')
            {
                eat(MUL);
                res = res * factor();
            }
            else if(opt.getValue() == '/')
            {
                eat(DIV);
                int right = factor();
                if(right != 0)
                {
                    res = res / right;
                }
                else throw ExceptionFactory("MathException","division by zero");
            }
            else throw ExceptionFactory("SyntaxException","invalid syntax");;
        }
        return res;
    }
}