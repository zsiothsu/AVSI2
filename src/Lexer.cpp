/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-12 11:20:24
 * @Description: file content
 */
#include "../inc/Lexer.h"

namespace AVSI
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
    Lexer::Lexer(std::string line)
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
            if(this->currentChar == ' ') { skipWhiteSpace(); continue; }
            if(isdigit(this->currentChar)) { return number(); }
            if(this->currentChar == '+') { advance(); return Token(add_opt,'+');}
            if(this->currentChar == '-') { advance(); return Token(dec_opt,'-'); }
            if(this->currentChar == '*') { advance(); return Token(mul_opt,'*'); }
            if(this->currentChar == '/') { advance(); return Token(div_opt,'/'); }
            if(this->currentChar == '(') { advance(); return Token(left_parenthese_keyword,'('); }
            if(this->currentChar == ')') { advance(); return Token(right_parenthese_keyword,')'); }
            if(this->currentChar == ';') { advance(); return Token(semi_keyword,';'); }
            if(isalpha(this->currentChar) || this->currentChar == '_') { std::string id = Id(); return Token(variable_ast,id); }
            if(this->currentChar == '=') {
                if(peek() != '=')
                {
                    advance();
                    return Token(assign_opt,')');
                }
                //TODO : eq
                else return Token::empty();
            }
            return Token::empty();
        }
        return Token(END,EOF);
    }

    /**
     * @description:    extract a (multidigit) number from the sentence, refered from cJson
     * @param:          None
     * @return:         a number Token
     */
    Token Lexer::number()
    {
        double num = 0,scale = 0;
        int subscale = 0,signsubscale = 1;
        
        if(this->currentChar == '0') advance(); // is zero
        if(this->currentChar >= '1' && this->currentChar <= '9') do { num = num * 10.0 + (this->currentChar - '0'); advance(); } while(this->currentChar >= '0' && this->currentChar <= '9'); // is number ?
        if(this->currentChar == '.' && peek() >= '0' && peek() <= '9')
        {
            advance();
            do
            {
                num = num * 10.0 + (this->currentChar - '0');
                scale--;
                advance();
            } while(this->currentChar >= '0' && this->currentChar <= '9');
        } // fractional part?
        if(this->currentChar == 'e' || this->currentChar == 'E')
        {
            advance(); if(this->currentChar == '+') advance(); else if(this->currentChar == '-') signsubscale = -1,advance();
            while(this->currentChar >= '0' && this->currentChar <= '9') subscale = subscale * 10 + (this->currentChar - '0'),advance();
        } // exponent?

        num = num * pow(10.0,(scale + subscale * signsubscale));
        if(scale == 0 && signsubscale == 1) return Token(integer_ast,(int)num);
        else return Token(float_ast,num);
    }

    char Lexer::peek()
    {
        if(this->cur + 1  < this->line.length())
        {
            return this->line[this->cur + 1];
        }
        else
        {
            return this->currentChar = EOF;
        }
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

    std::string Lexer::Id()
    {
        std::string str;
        while (
            isalpha(this->currentChar) ||
            isdigit(this->currentChar) ||
            this->currentChar == '_'
        )
        {
            str = str + this->currentChar;
            advance();
        }
        return str;
    }
}