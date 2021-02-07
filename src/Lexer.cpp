/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:56:33
 * @Description: include Lexer class
 */
#include "../inc/Lexer.h"

namespace AVSI {
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    /**
     * @description:    default constructor
     * @param:          None
     * @return:         None
     */
    Lexer::Lexer(void) {}

    Lexer::Lexer(ifstream *file) {
        this->file = file;
        this->linenum = 1;
        this->cur = 0;
        do {
            getline(*this->file, this->line);
            this->line += " ";
            this->currentChar = this->line[this->cur];
        } while (this->line.empty());
    }

    /**
     * @description:    default destructor
     * @param:          None
     * @return:         None
     */
    Lexer::~Lexer() {}

    /*******************************************************
     *                  lexical analyzer                   *
     *******************************************************/
    /**
     * @description:    advance the "cur" pointer and set the "currentChar"
     * variable
     * @param:          None
     * @return:         None
     */
    void Lexer::advance() {
        this->cur += 1;
        if (this->cur < this->line.length()) {
            this->currentChar = this->line[this->cur];
        } else {
            do {
                if (!this->file->eof()) {
                    this->linenum++;
                    this->cur = 0;
                    getline(*this->file, this->line);
                    this->line += " ";
                    this->currentChar = this->line[this->cur];
                } else {
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
    Token Lexer::getNextToken() {
        while (this->currentChar != EOF) {
            int line = this->linenum, column = this->cur;
            if (this->currentChar == ' ') {
                skipWhiteSpace();
                continue;
            }
            if (this->currentChar == '#') {
                this->cur = this->line.length();
                advance();
                continue;
            }
            if (isdigit(this->currentChar)) { return number(); }
            if (isalpha(this->currentChar) || this->currentChar == '_') {
                return Id();
            }
            if (this->currentChar == '"') { return str(); }
            if (this->currentChar == '|') {
                if (peek() == '|') {
                    advance(); advance();
                    return Token(OR, 'o', line, column);
                }
                return Token::empty();
            }
            if (this->currentChar == '&') {
                if (peek() == '&') {
                    advance(); advance();
                    return Token(AND, 'o', line, column);
                }
                return Token::empty();
            }
            if (this->currentChar == '=') {
                if (peek() != '=') {
                    advance();
                    return Token(EQUAL, '=', line, column);
                }
                else if(peek() == '=') {
                    advance();
                    advance();
                    return Token(EQ,'o',line,column);
                }
                else
                    return Token::empty();
            }
            if (this->currentChar == '!') {
                if (peek() != '=') {
                    advance();
                    return Token(NOT, '!', line, column);
                }
                if(peek() == '=') {
                    advance();
                    advance();
                    return Token(NE,'o',line,column);
                }
                else
                    return Token::empty();
            }
            if (this->currentChar == '>') {
                if (peek() != '=') {
                    advance();
                    return Token(GT, 'o', line, column);
                }
                if(peek() == '=') {
                    advance();
                    advance();
                    return Token(GE,'o',line,column);
                }
                else
                    return Token::empty();
            }
            if (this->currentChar == '<') {
                if (peek() != '=') {
                    advance();
                    return Token(LT, '!', line, column);
                }
                if(peek() == '=') {
                    advance();
                    advance();
                    return Token(LE,'o',line,column);
                }
                else
                    return Token::empty();
            }
            if (this->currentChar == '-') {
                char _peek = peek();
                string _peek2 = peek2();
                if(_peek2 == "eq") {advance(); advance(); advance(); return Token(EQ,'o',line,column);}
                if(_peek2 == "ne") {advance(); advance(); advance(); return Token(NE,'o',line,column);}
                if(_peek2 == "gt") {advance(); advance(); advance(); return Token(GT,'o',line,column);}
                if(_peek2 == "lt") {advance(); advance(); advance(); return Token(LT,'o',line,column);}
                if(_peek2 == "ge") {advance(); advance(); advance(); return Token(GE,'o',line,column);}
                if(_peek2 == "le") {advance(); advance(); advance(); return Token(LE,'o',line,column);}
                if(_peek == 'o') {advance(); advance(); return Token(OR,'o',line,column);}
                if(_peek == 'a') {advance(); advance(); return Token(AND,'o',line,column);}
            }
            map<char, TokenType>::iterator iter =
                    TokenMap.find(this->currentChar);
            if (iter != TokenMap.end()) {
                char tokenVaule = this->currentChar;
                advance();
                return Token(iter->second, tokenVaule, line, column);
            }
            return Token::empty();
        }
        return Token(END, EOF);
    }

    /**
     * @description:    extract a (multidigit) number from the sentence, refered
     * from cJson
     * @param:          None
     * @return:         a number Token
     */
    Token Lexer::number() {
        int line = this->linenum, column = this->cur;
        double num = 0, scale = 0;
        int subscale = 0, signsubscale = 1;

        if (this->currentChar == '0') advance(); // is zero
        if (this->currentChar >= '1' && this->currentChar <= '9')
            do {
                num = num * 10.0 + (this->currentChar - '0');
                advance();
            } while (this->currentChar >= '0' &&
                     this->currentChar <= '9'); // is number ?
        if (this->currentChar == '.' && peek() >= '0' && peek() <= '9') {
            advance();
            do {
                num = num * 10.0 + (this->currentChar - '0');
                scale--;
                advance();
            } while (this->currentChar >= '0' && this->currentChar <= '9');
        } // fractional part?
        if (this->currentChar == 'e' || this->currentChar == 'E') {
            advance();
            if (this->currentChar == '+')
                advance();
            else if (this->currentChar == '-')
                signsubscale = -1, advance();
            while (this->currentChar >= '0' && this->currentChar <= '9')
                subscale = subscale * 10 + (this->currentChar - '0'), advance();
        } // exponent?

        num = num * pow(10.0, (scale + subscale * signsubscale));
        if (scale == 0 && signsubscale == 1)
            return Token(INTEGER, (int) num, line, column);
        else
            return Token(FLOAT, num, line, column);
    }

    char Lexer::peek() {
        if (this->cur + 1 < this->line.length()) {
            return this->line[this->cur + 1];
        } else {
            return this->currentChar = 0;
        }
    }

    string Lexer::peek2() {
        char first = peek();
        char second;

        if (this->cur + 2 < this->line.length()) {
            second = this->line[this->cur + 2];
        } else {
            second = this->currentChar = 0;
        }

        return string({first,second});
    }

    /**
     * @description:    skip white space
     * @param:          None
     * @return:         None
     */
    void Lexer::skipWhiteSpace() {
        while ((this->currentChar != EOF) && (this->currentChar == ' ')) {
            advance();
        }
    }

    Token Lexer::str() {
        int line = this->linenum, column = this->cur;
        std::string str;
        advance();
        while (this->currentChar != '"') {
            str = str + this->currentChar;
            advance();
        }
        advance();
        return Token(STRING, str, line, column);
    }

    Token Lexer::Id() {
        int line = this->linenum, column = this->cur;
        std::string str;
        while (isalpha(this->currentChar) || isdigit(this->currentChar) ||
               this->currentChar == '_') {
            str = str + this->currentChar;
            advance();
        }
        map<string, TokenType>::iterator iter = reservedKeyword.find(str);
        if (iter != reservedKeyword.end())
            return Token(iter->second, str, line, column);
        else
            return Token(ID, str, line, column);
    }
} // namespace AVSI