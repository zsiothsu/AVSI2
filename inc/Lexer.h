/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:51:01
 * @Description: include Lexer class
 */
#ifndef ___LEXER_H___
#define ___LEXER_H___

#include "AST.h"
#include <cstdlib>
#include <fstream>

namespace AVSI {
    class Lexer
    {
      private:
        ifstream* file;
        unsigned int linenum;
        unsigned int cur;
        std::string line;

      public:
        char currentChar;

        Lexer(void);
        Lexer(ifstream* file);
        ~Lexer();

        void advance();
        Token getNextToken();
        Token number();
        char peek();
        void skipWhiteSpace();
        Token Id();
    };

    static map<char, TokenType> TokenMap = {{'+', PLUS},
                                            {'-', MINUS},
                                            {'*', STAR},
                                            {'/', SLASH},
                                            {'(', LPAR},
                                            {')', RPAR},
                                            {'[', LSQB},
                                            {']', RSQB},
                                            {'{', LBRACE},
                                            {'}', RBRACE},
                                            {';', SEMI},
                                            {',', COMMA}};

    static map<string, TokenType> reservedKeyword = {
        {"function", FUNCTION},
        {"return",RETURN}};
} // namespace AVSI

#endif