/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-12 11:15:00
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
            if(currentToken.getType() == left_parenthese_keyword) this->parenCnt++;
            if(currentToken.getType() == right_parenthese_keyword) this->parenCnt--;
            this->currentToken = this->lexer->getNextToken();
        }
        else
        {
            throw ExceptionFactory("SyntaxException","invalid syntax");
        }
    }

    AST* Parser::program()
    {
        return statementList();
    }

    AST* Parser::statementList()
    {
        Compound* root = new Compound();
        AST* node;
        while((node = statement()) != &ASTEmpty) root->child.push_back(node);
        return root;
    }

    AST* Parser::statement()
    {
        AST* ret = &ASTEmpty;
        if(this->currentToken.getType() == variable_ast) ret = assignment();
        return ret;
    }

    AST* Parser::assignment()
    {
        AST* left = variable();
        eat(assign_opt);
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
            while(this->currentToken.getType() == add_opt || \
                  this->currentToken.getType() == dec_opt)
            {
                Token opt = this->currentToken;
                if(opt.getValue() == '+')
                {
                    eat(add_opt);
                    res = new BinOp(res,opt,term());
                }
                else if(opt.getValue() == '-')
                {
                    eat(dec_opt);
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
            if(token.getType() == integer_ast || token.getType() == float_ast) { eat(token.getType()); return new Num(token); }
            if(token.getType() == add_opt) { eat(add_opt); return new UnaryOp(new Num(Token(integer_ast,0)),token,factor()); }
            if(token.getType() == dec_opt) { eat(dec_opt); return new UnaryOp(new Num(Token(integer_ast,0)),token,factor()); }
            if(token.getType() == variable_ast) { return variable(); }
            if(token.getType() == left_parenthese_keyword) { eat(left_parenthese_keyword); AST* res = expr(); eat(right_parenthese_keyword); return res; }
            if(token.getType() == right_parenthese_keyword)
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
            return program();   
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
            while(this->currentToken.getType() == mul_opt || \
                  this->currentToken.getType() == div_opt)
            {
                Token opt = this->currentToken;
                if(opt.getValue() == '*')
                {
                    eat(mul_opt);
                    res = new BinOp(res,opt,factor());
                }
                else if(opt.getValue() == '/')
                {
                    eat(div_opt);
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
            eat(variable_ast);
            return new Variable(var);
        }
        catch(Exception& e)
        {
            throw e;
        }
    }
}