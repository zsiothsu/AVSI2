/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 18:20:12
 * @Description: file content
 */
#include "../inc/Lexer.h"

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
    Lexer::Lexer(void)
    {
    }
    
    /**
     * @description:    to initialize a Lexer.
     * @param:          line:command to be executed
     * @return:         None
     */
    Lexer::Lexer(string line)
    {
        this->line = line;
        this->cur = 0;
        this->currentChar = this->line[0];
    }

    /**
     * @description:    default destructor
     * @param:          None
     * @return:         None
     */
    Lexer::~Lexer()
    {
    }

    /*******************************************************
     *                  lexical analyzer                   *
     *******************************************************/
    /**
     * @description:    advance the "cur" pointer and set the "currentChar" variable
     * @param:          None
     * @return:         None
     */
    void Lexer::advance()
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
    Token Lexer::getNextToken()
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
            if(this->currentChar == '(')
            {
                advance();
                return Token(LPAREN,'(');
            }
            if(this->currentChar == ')')
            {
                advance();
                return Token(RPAREN,')');
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
    int Lexer::integer()
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
    void Lexer::skipWhiteSpace()
    {
        while((this->currentChar != EOF) && (this->currentChar == ' '))
        {
            advance();
        }
    }
}