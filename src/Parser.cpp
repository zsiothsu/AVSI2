/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 21:02:29
 * @Description: file content
 */
#include "../inc/Parser.h"

namespace INTERPRETER
{
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    Parser::Parser(void)
    {
    }

    Parser::Parser(Lexer& lexer)
    {
        this->lexer = &lexer;
        this->currentToken = lexer.getNextToken();
    }

    Parser::~Parser()
    {
    }

    /*******************************************************
     *                         parser                      *
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
    void Parser::eat(CharType type)
    {
        if(type == currentToken.getType())
        {
            this->currentToken = this->lexer->getNextToken();
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
    AST* Parser::expr(void)
    {
        try
        {
            AST* res = term();
            while(this->currentToken.getType() == ADD || \
                  this->currentToken.getType() == DEC)
            {
                Token opt = this->currentToken;
                if(opt.getValue() == '+')
                {
                    eat(ADD);
                    res = new BinOp(res,opt,term());
                }
                else if(opt.getValue() == '-')
                {
                    eat(DEC);
                    res = new BinOp(res,opt,term());
                }
                else throw ExceptionFactory("SyntaxException","invalid syntax");
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
     * @return:         factor
     * @grammar:        integer | LPAREN
     */
    //TODO: solve throwed Exception when input "()" without expr
    AST* Parser::factor(void)
    {
        Token token = this->currentToken;
        if(token.getType() == INT)
        {
            eat(INT);
            return new Num(token);
        }
        else if(token.getType() == LPAREN)
        {
            try
            {
                eat(LPAREN);
                AST* res = expr();
                eat(RPAREN);
                return res;   
            }
            catch(Exception& e)
            {
                throw e;   
            }
        }
        else throw ExceptionFactory("SyntaxException","invalid syntax");
    }

    /**
     * @description:    parser entry
     * @param:          None
     * @return:         root of parse result.
     */
    AST* Parser::parse(void)
    {
        try
        {
            return expr();   
        }
        catch(Exception& e)
        {
            throw e;
        }
    }

    /**
     * @description:    return MUL / DIV result 
     * @param:          None
     * @return:         term for expr
     * @throw           SyntaxException
     * @grammar:        term: factor ((MUL | DIV) factor)*
     *                  factor: integer
     */
    AST* Parser::term(void)
    {
        try
        {
            AST* res = factor();
            while(this->currentToken.getType() == MUL || \
                  this->currentToken.getType() == DIV)
            {
                Token opt = this->currentToken;
                if(opt.getValue() == '*')
                {
                    eat(MUL);
                    res = new BinOp(res,opt,factor());
                }
                else if(opt.getValue() == '/')
                {
                    eat(DIV);
                    res = new BinOp(res,opt,factor());
                }
                else throw ExceptionFactory("SyntaxException","invalid syntax");
            }
            return res;
        }
        catch(Exception& e)
        {
            throw e;
        }
    }
}