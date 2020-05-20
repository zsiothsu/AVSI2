/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-20 11:38:22
 * @Description: include Lexer class
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

    Lexer::Lexer(ifstream* file)
    {
        this->file = file;
        this->linenum = 1;
        this->cur = 0;
        do
        {
            getline(*this->file,this->line);
            this->currentChar = this->line[this->cur];
        } while(this->line.empty());
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
            do
            {
                if(!this->file->eof())
                {
                    this->linenum++;
                    this->cur = 0;
                    getline(*this->file,this->line);
                    this->currentChar = this->line[this->cur];
                }
                else
                {
                    this->currentChar = EOF;
                    return;
                }
            } while (this->line.empty());
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
            if(this->currentChar == '#') { this->cur = this->line.length(); advance(); continue; }
            if(isdigit(this->currentChar)) { return number(); }
            if(isalpha(this->currentChar) || this->currentChar == '_') { return Id(); }
            if(this->currentChar == '=') {
                if(peek() != '=')
                {
                    advance();
                    return Token(assign_opt,')');
                }
                //TODO : eq
                else return Token::empty();
            }
            map<char,TokenType>::iterator iter = TokenMap.find(this->currentChar);
            if(iter != TokenMap.end())
            {
                advance();
                return Token(iter->second,this->currentChar);
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
            return this->currentChar = 0;
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

    Token Lexer::Id()
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
        map<string,TokenType>::iterator iter = reservedKeyword.find(str);
        if(iter != reservedKeyword.end()) return Token(iter->second,str);
        else return Token(id_ast,str);
    }
}