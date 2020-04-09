/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-04-09 14:13:56
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
            if(this->currentChar == '+' || \
               this->currentChar == '-')
            {
                char opt = this->currentChar;
                advance();
                return Token(OPT,opt);
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
     * @description:    implement an arithmetic expression for integer
     * @param:          left: left value
     * @param:          right: right value
     * @param:          opt: operator
     * @return:         result of arithmetic expression
     * @throw:          SyntaxException
     */
    int Interpreter::calc(int left,int right,Token opt)
    {
        char tmp = opt.getValue();
        switch (tmp)
        {
        case '+':
            return left + right;
            break;
        case '-':
            return left - right;
            break;
        case '*':
            return left * right;
            break;
        case '/':
            return left / right;
            break;
        default:
            break;
        }
        throw SyntaxException("invalid syntax");
    }

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
            throw SyntaxException("invalid syntax");
        }
    }

    /**
     * @description:    arithmetic expression parser / interpreter.
     * @param:          None
     * @return:         result of arithmetic expression
     * @throw:          SyntaxException
     * @grammar:        expr   : factor ((ADD | DEC) factor)*
     *                  factor : INTEGER
     */
    int Interpreter::expr()
    {
        try{
            this->currentToken = getNextToken();
            int res = factor();

            while(this->currentToken.getType() == OPT)
            {
                Token opt = this->currentToken;
                eat(OPT);
                int right = factor();

                res = calc(res,right,opt);
            }

            return res;
        }
        catch(SyntaxException e)
        {
            throw e;
        }
    }

    /**
     * @description:    return an INT token value
     * @param:          None
     * @return:         integer
     */
    int Interpreter::factor()
    {
        Token token = this->currentToken;
        eat(INT);
        return token.getValue();
    }
}