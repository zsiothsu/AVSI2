/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-28 17:18:57
 * @Description: include Parser class
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
            throw ExceptionFactory(__SyntaxException,"invalid syntax",this->currentToken.line,this->currentToken.column);
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
        if(this->currentToken.getType() == function_keyword)
        {
            return functionDecl();
        }
        else if(this->currentToken.getType() == id_ast && this->lexer->currentChar == '(')
        {
            AST* ast = functionCall();
            return ast;
        }
        else if(this->currentToken.getType() == id_ast)
        {
            return assignment();
        }
        return &ASTEmpty;
    }

    AST* Parser::assignment()
    {
        AST* left = variable();
        eat(assign_opt);
        AST* right = expr();
        return new Assign(left,right);
    }

    AST* Parser::functionDecl()
    {
        Token token = this->currentToken;
        eat(function_keyword);
        string id = this->currentToken.getValue().any_cast<string>();
        eat(id_ast);

        AST* paramList = nullptr;
        eat(left_parenthese_keyword);
        if(this->currentToken.getType() == id_ast) paramList = param();
        eat(right_parenthese_keyword);

        eat(left_brace_keyword);
        AST* compound = statementList();
        eat(right_brace_keyword);
        return new FunctionDecl(id,paramList,compound,token);
    }

    AST* Parser::functionCall()
    {
        Token token = this->currentToken;
        string id = this->currentToken.getValue().any_cast<string>();
        eat(id_ast);

        vector<AST*> paramList;
        eat(left_parenthese_keyword);
        if(this->currentToken.getType() != right_parenthese_keyword)
        {
            paramList.push_back(expr());
            while(this->currentToken.getType() == comma_keyword)
            {
                eat(comma_keyword);
                paramList.push_back(expr());
            }
        }
        eat(right_parenthese_keyword);
        FunctionCall* fun = new FunctionCall(id,paramList,token);
        return fun;
    }

    AST* Parser::param()
    {
        Param* param = new Param();
        set<string> paramSet;
        while(this->currentToken.getType() == id_ast)
        {
            string id = this->currentToken.getValue().any_cast<string>();
            if(paramSet.find(id) != paramSet.end())
            {
                string msg = "duplicate argument '" + id +"' in function definition";
                throw ExceptionFactory(__SyntaxException,msg,this->currentToken.line,this->currentToken.column);
            }
            paramSet.insert(id);
            param->paramList.push_back(new Variable(this->currentToken));
            eat(id_ast);
            if(this->currentToken.getType() == right_parenthese_keyword)
            {
                return param;
            }
            eat(comma_keyword);
        }
        throw ExceptionFactory(__SyntaxException,"unexpected symbol in parameter list",this->currentToken.line,this->currentToken.column);
        return param;
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
            else throw ExceptionFactory(__SyntaxException,"unrecognized operator",opt.line,opt.column);
        }
        return res; 
    }

    /**
     * @description:    return an Integer factor
     * @param:          none
     * @return:         factor
     * @grammar:        Integer | LPAREN
     */
    AST* Parser::factor(void)
    {
        Token token = this->currentToken;
        if(token.getType() == integer_ast || token.getType() == float_ast) { eat(token.getType()); return new Num(token); }
        if(token.getType() == add_opt) { eat(add_opt); return new UnaryOp(token,factor()); }
        if(token.getType() == dec_opt) { eat(dec_opt); return new UnaryOp(token,factor()); }
        if(token.getType() == id_ast) { return variable(); }
        if(token.getType() == left_parenthese_keyword) { eat(left_parenthese_keyword); AST* res = expr(); eat(right_parenthese_keyword); return res; }
        if(token.getType() == right_parenthese_keyword)
        {
            if(this->parenCnt <= 0) throw ExceptionFactory(__SyntaxException,"unmatched ')'",token.line,token.column);
            return new NoneAST();
        }
        else throw ExceptionFactory(__SyntaxException,"unrecognized factor in expression",token.line,token.column);
    }

    /**
     * @description:    parser entry
     * @param:          None
     * @return:         root of parse result.
     */
    AST* Parser::parse(void)
    {
        return program();   
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
            else throw ExceptionFactory(__SyntaxException,"unrecognized operator",opt.line,opt.column);
        }
        return res;
    }

    AST* Parser::variable()
    {
        Token var = this->currentToken;
        eat(id_ast);
        return new Variable(var);
    }
}