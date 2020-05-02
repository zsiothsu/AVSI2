/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-01 19:26:51
 * @Description: file content
 */
#include "../inc/Parser.h"

namespace AVSI
{
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    Parser::Parser(void)
    {
    }

    Parser::Parser(Lexer* lexer)
    {
        this->lexer = lexer;
        this->currentToken = lexer->getNextToken();
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
    void Parser::eat(TokenType type)
    {
        if(type == currentToken.getType())
        {
            if(currentToken.getType() == LPAREN) this->parenCnt++;
            if(currentToken.getType() == RPAREN) this->parenCnt--;
            this->currentToken = this->lexer->getNextToken();
        }
        else
        {
            throw ExceptionFactory("SyntaxException","invalid syntax");
        }
    }

    AST* Parser::assignment()
    {
        AST* left = variable();
        eat(ASSIGN);
        AST* right = expr();
        return new Assign(left,right);
    }

    /**
     * @description:    arithmetic expression parser / interpreter.
     * @param:          None
     * @return:         result of arithmetic expression
     * @throw:          SyntaxException
     * @grammar:        expr: term ((ADD | DEC) term)*
     *                  term: Integer;
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
                if(opt.getChar() == '+')
                {
                    eat(ADD);
                    res = new BinOp(res,opt,term());
                }
                else if(opt.getChar() == '-')
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
     * @description:    return an Integer factor
     * @param:          none
     * @return:         factor
     * @grammar:        Integer | LPAREN
     */
    AST* Parser::factor(void)
    {
        try
        {
            Token token = this->currentToken;
            if(token.getType() == INT || token.getType() == FLT) { eat(token.getType()); return new Num(token); }
            if(token.getType() == ADD) { eat(ADD); return new UnaryOp(new Num(Token(INT,0)),token,factor()); }
            if(token.getType() == DEC) { eat(DEC); return new UnaryOp(new Num(Token(INT,0)),token,factor()); }
            if(token.getType() == VAR) { return variable(); }
            if(token.getType() == LPAREN) { eat(LPAREN); AST* res = expr(); eat(RPAREN); return res; }
            if(token.getType() == RPAREN)
            {
                if(this->parenCnt <= 0) throw ExceptionFactory("SyntaxException","unmatched ')'");
                return new NoneAST();
            }
            else throw ExceptionFactory("SyntaxException","invalid syntax");
        }
        catch(Exception& e)
        {
            throw e;
        }
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
     * @c:        term: factor ((MUL | DIV) factor)*
     *                  factor: Integer
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
                if(opt.getChar() == '*')
                {
                    eat(MUL);
                    res = new BinOp(res,opt,factor());
                }
                else if(opt.getChar() == '/')
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

    AST* Parser::variable()
    {
        try
        {
            Token var = this->currentToken;
            eat(VAR);
            return new Variable(var);
        }
        catch(Exception& e)
        {
            throw e;
        }
    }
}