/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-28 17:18:57
 * @Description: include Parser class
 */
#include "../inc/Parser.h"

namespace AVSI {
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    Parser::Parser(void) {}

    Parser::Parser(Lexer *lexer) {
        this->lexer = lexer;
        this->currentToken = lexer->getNextToken();
    }

    Parser::~Parser() {}

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
    void Parser::eat(TokenType type) {
        if (type == currentToken.getType()) {
            if (currentToken.getType() == LPAR)
                this->parenCnt++;
            if (currentToken.getType() == RPAR)
                this->parenCnt--;
            this->currentToken = this->lexer->getNextToken();
        } else {
            throw ExceptionFactory(__SyntaxException, "invalid syntax",
                                   this->currentToken.line,
                                   this->currentToken.column);
        }
    }

    AST *Parser::program() { return statementList(); }

    AST *Parser::statementList() {
        Compound *root = new Compound();
        AST *node;
        while ((node = statement()) != &ASTEmpty) root->child.push_back(node);
        return root;
    }

    AST *Parser::statement() {
        if (this->currentToken.getType() == FUNCTION) {
            return functionDecl();
        } else if (this->currentToken.getType() == RETURN) {
            return returnExpr();
        } else if (this->currentToken.getType() == ECHO) {
            return echo();
        } else if (this->currentToken.getType() == PRINTF) {
            return print();
        } else if (this->currentToken.getType() == ID &&
                   this->lexer->currentChar == '(') {
            AST *ast = functionCall();
            return ast;
        } else if (this->currentToken.getType() == ID) {
            return assignment();
        } else if (this->currentToken.getType() == IF) {
            return IfStatement();
        } else if (this->currentToken.getType() == FOR) {
            return forStatement();
        } else if (this->currentToken.getType() == WHILE) {
            return WhileStatement();
        } else if (this->currentToken.getType() == GLOBAL) {
            return global();
        }
        return &ASTEmpty;
    }

    AST *Parser::assignment() {
        AST *left = variable();
        eat(EQUAL);
        AST *right = expr();
        return new Assign(left, right);
    }

    AST *Parser::forStatement() {
        Token token = this->currentToken;

        AST *initList = nullptr;
        AST *condition = nullptr;
        AST *adjustment = nullptr;
        AST *compound = nullptr;
        bool noCondition = false;

        eat(FOR);

        eat(LPAR);
        initList = statementList();
        eat(SEMI);
        if (this->currentToken.getType() == SEMI)
            noCondition = true;
        else
            condition = expr();
        eat(SEMI);
        adjustment = statementList();
        eat(RPAR);

        eat(DO);
        compound = statementList();
        eat(DONE);

        return new For(initList, condition, adjustment, compound, noCondition, token);
    }

    AST *Parser::functionDecl() {
        Token token = this->currentToken;
        eat(FUNCTION);
        string id = this->currentToken.getValue().any_cast<string>();
        eat(ID);

        AST *paramList = nullptr;
        eat(LPAR);
        paramList = param();
        eat(RPAR);

        if (this->currentToken.getType() == LBRACE) {
            eat(LBRACE);
            AST *compound = statementList();
            eat(RBRACE);
            return new FunctionDecl(id, paramList, compound, token);
        } else {
            return new FunctionDecl(id, paramList, nullptr, token);
        }
    }

    AST *Parser::functionCall() {
        Token token = this->currentToken;
        string id = this->currentToken.getValue().any_cast<string>();
        eat(ID);

        vector<AST *> paramList;
        eat(LPAR);
        if (this->currentToken.getType() != RPAR) {
            paramList.push_back(expr());
            while (this->currentToken.getType() == COMMA) {
                eat(COMMA);
                paramList.push_back(expr());
            }
        }
        eat(RPAR);
        FunctionCall *fun = new FunctionCall(id, paramList, token);
        return fun;
    }

    AST *Parser::global() {
        Token token = this->currentToken;
        eat(GLOBAL);

        AST *var = variable();

        return new Global(var, token);
    }

    AST *Parser::IfStatement() {
        Token token = this->currentToken;
        TokenType type = this->currentToken.getType();
        AST *condition = nullptr;
        bool noCondition = false;

        if (type == FI) {
            eat(FI);
            return &ASTEmpty;
        }

        if (type == IF) {
            eat(IF);
            eat(LSQB);
            condition = expr();
            eat(RSQB);
            eat(THEN);
        } else if (type == ELIF) {
            eat(ELIF);
            eat(LSQB);
            condition = expr();
            eat(RSQB);
            eat(THEN);
        } else {
            eat(ELSE);
            noCondition = true;
        }

        AST *compound = statementList();
        AST *next = IfStatement();

        return new If(condition, noCondition, compound, next, token);
    }

    AST *Parser::param() {
        Param *param = new Param();
        set<string> paramSet;
        while (this->currentToken.getType() == ID) {
            string id = this->currentToken.getValue().any_cast<string>();
            if (paramSet.find(id) != paramSet.end()) {
                string msg =
                        "duplicate argument '" + id + "' in function definition";
                throw ExceptionFactory(__SyntaxException, msg,
                                       this->currentToken.line,
                                       this->currentToken.column);
            }
            paramSet.insert(id);
            param->paramList.push_back(new Variable(this->currentToken));
            eat(ID);
            if (this->currentToken.getType() == RPAR) {
                return param;
            }
            eat(COMMA);
        }
        if (this->currentToken.getType() == RPAR) {
            return param;
        }
        throw ExceptionFactory(
                __SyntaxException, "unexpected symbol in parameter list",
                this->currentToken.line, this->currentToken.column);
        return param;
    }

    AST *Parser::echo(void) {
        Token token = this->currentToken;
        eat(ECHO);

        AST *content = expr();

        return new Echo(content, token);
    }

    AST *Parser::print() {
        Token token = this->currentToken;
        eat(PRINTF);

        /* TODO: expr() -> string() */
        AST *content = expr();

        return new Printf(content, token);
    }

    /**
     * @description:    arithmetic expression parser / interpreter.
     * @param:          None
     * @return:         result of arithmetic expression
     * @throw:          SyntaxException
     * @grammar:        expr: term ((ADD | DEC) term)*
     *                  term: Integer;
     */
    AST *Parser::expr(void) {
        if (this->currentToken.getType() == SEMI) return &ASTEmpty;

        AST *res = term();
        while (this->currentToken.getType() == PLUS ||
               this->currentToken.getType() == MINUS) {
            Token opt = this->currentToken;
            if (opt.getValue() == '+') {
                eat(PLUS);
                res = new BinOp(res, opt, term());
            } else if (opt.getValue() == '-') {
                eat(MINUS);
                res = new BinOp(res, opt, term());
            } else
                throw ExceptionFactory(__SyntaxException,
                                       "unrecognized operator", opt.line,
                                       opt.column);
        }

        while (this->currentToken.isReOp()) {
            Token opt = this->currentToken;
            eat(opt.getType());
            res = new BinOp(res, opt, expr());
        }

        return res;
    }

    /**
     * @description:    return an Integer factor
     * @param:          none
     * @return:         factor
     * @grammar:        Integer | LPAREN
     */
    AST *Parser::factor(void) {
        Token token = this->currentToken;
        if (token.getType() == STRING) {
            eat(STRING);
            return new class String(token);
        }
        if (token.getType() == INTEGER || token.getType() == FLOAT) {
            eat(token.getType());
            return new Num(token);
        }
        if (token.getType() == TRUE || token.getType() == FALSE) {
            eat(token.getType());
            return new Boolean(token);
        }
        if (token.getType() == PLUS) {
            eat(PLUS);
            return new UnaryOp(token, factor());
        }
        if (token.getType() == MINUS) {
            eat(MINUS);
            return new UnaryOp(token, factor());
        }
        if (token.getType() == NOT) {
            eat(NOT);
            return new UnaryOp(token, factor());
        }
        if (token.getType() == ID && this->lexer->currentChar == '(') {
            return functionCall();
        }
        if (token.getType() == ID || token.getType() == DOLLAR) { return variable(); }
        if (token.getType() == LPAR) {
            eat(LPAR);
            AST *res = expr();
            eat(RPAR);
            return res;
        }
        if (token.getType() == RPAR) {
            if (this->parenCnt <= 0)
                throw ExceptionFactory(__SyntaxException, "unmatched ')'",
                                       token.line, token.column);
            return new NoneAST();
        } else
            throw ExceptionFactory(__SyntaxException,
                                   "unrecognized factor in expression",
                                   token.line, token.column);
    }

    /**
     * @description:    parser entry
     * @param:          None
     * @return:         root of parse result.
     */
    AST *Parser::parse(void) { return program(); }

    AST *Parser::returnExpr(void) {
        Token token = this->currentToken;
        eat(RETURN);

        AST *ret = nullptr;
        if (this->currentToken.isExpr()) ret = expr();

        return new Return(token, ret);
    }

    /**
     * @description:    return MUL / DIV result
     * @param:          None
     * @return:         term for expr
     * @throw           SyntaxException
     * @c:        term: factor ((MUL | DIV) factor)*
     *                  factor: Integer
     */
    AST *Parser::term(void) {
        AST *res = factor();
        while (this->currentToken.getType() == STAR ||
               this->currentToken.getType() == SLASH) {
            Token opt = this->currentToken;
            if (opt.getValue() == '*') {
                eat(STAR);
                res = new BinOp(res, opt, factor());
            } else if (opt.getValue() == '/') {
                eat(SLASH);
                res = new BinOp(res, opt, factor());
            } else
                throw ExceptionFactory(__SyntaxException,
                                       "unrecognized operator", opt.line,
                                       opt.column);
        }
        return res;
    }

    AST *Parser::variable() {
        if (this->currentToken.getType() == DOLLAR)
            eat(DOLLAR);

        Token var = this->currentToken;
        eat(ID);
        return new Variable(var);
    }

    AST *Parser::WhileStatement() {
        Token token = this->currentToken;
        AST *condition;
        AST *compound;

        eat(WHILE);
        condition = expr();
        eat(DO);
        compound = statementList();
        eat(DONE);

        return new While(condition, compound, token);
    }

} // namespace AVSI