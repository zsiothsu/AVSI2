/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-20 11:06:41
 * @Description: definition of tokens or interpreter
 */
#ifndef ___TOKEN_H___
#define ___TOKEN_H___

#include "any.h"

namespace AVSI
{
    using namespace std;
    typedef enum
    {
        //special token
        END = EOF,
        NONE = 0,
        // terminal
        integer_ast,
        float_ast,
        compound_ast,
        id_ast,
        // operator
        add_opt,
        dec_opt,
        mul_opt,
        div_opt,
        assign_opt,
        // symbol
        left_parenthese_keyword,
        right_parenthese_keyword,
        left_bracket_keyword,
        right_bracket_keyword,
        left_brace_keyword,
        right_brace_keyword,
        semi_keyword,
        // reserved keywork
        function_keyword
    } TokenType;
    
    typedef char opt;
    
    class Token
    {
    private:
        any         value;
        TokenType   type;
    public:
        Token();
        Token(TokenType type,any var);
        ~Token();

        static Token empty();

        TokenType getType();
        any getValue();
        any getNum();
        char getChar();
        std::string getString();
        //std::string __str();
    };
    
    const static Token emptyToken(NONE,0);

    std::string typeName(TokenType type);
}

#endif
