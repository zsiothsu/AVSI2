/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-28 17:18:57
 * @Description: include Parser class
 */
#include "../inc/Parser.h"
#include "SymbolTable.h"

namespace AVSI {
    /*******************************************************
     *                     variable                        *
     *******************************************************/
    extern llvm::LLVMContext *the_context;
    extern llvm::Module *the_module;
    extern string module_name;
    extern vector<string> module_path;

    extern map<string, StructDef *> struct_types;

    extern map<llvm::Type *, string> type_name;
    extern map<llvm::Type *, uint32_t> type_size;

    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    Parser::Parser(void) {}

    Parser::Parser(Lexer *lexer) {
        this->lexer = lexer;
        this->currentToken = lexer->getNextToken();
        this->lastToken = this->currentToken;
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
            this->lastToken = this->currentToken;
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
        bool is_export = false;
        static bool mod_named = false;

        if (this->currentToken.getType() == EXPORT) {
            is_export = true;
            eat(EXPORT);
        }
        if (this->currentToken.getType() == MODULE) {
            if (mod_named) {
                Warning(
                        __Warning,
                        "Module has named before.",
                        this->currentToken.line, this->currentToken.column
                );
            }

            eat(MODULE);
            if (this->currentToken.getType() == ID) {
                vector<string> path = this->currentToken.getModInfo();
                module_path = path;
                path.push_back(this->currentToken.getValue().any_cast<string>());
                module_name = getModuleNameByPath(path);
                mod_named = true;
                the_module->setModuleIdentifier(module_name);
            }
            eat(ID);
            return &ASTEmptyNotEnd;
        }


        TokenType token_type = this->currentToken.getType();

        if (token_type == FUNCTION) {
            FunctionDecl *may_be_export = (FunctionDecl *) functionDecl();
            may_be_export->is_export = is_export;
            return may_be_export;
        } else if (token_type == RETURN) {
            return returnExpr();
        } else if (token_type == ID &&
                   this->lexer->currentChar == '(') {
            AST *ast = functionCall();
            return ast;
        } else if (token_type == ID) {
            return assignment();
        } else if (token_type == IF) {
            return IfStatement();
        } else if (token_type == FOR) {
            return forStatement();
        } else if (token_type == WHILE) {
            return WhileStatement();
        } else if (token_type == GLOBAL) {
            Global *may_be_export = (Global *) global();
            may_be_export->is_export = is_export;
            return may_be_export;
        } else if (token_type == OBJ) {
            Object *may_be_export = (Object *) object();
            may_be_export->is_export = is_export;
            return may_be_export;
        } else if (token_type == LBRACE) {
            return arraylist();
        }

        return &ASTEmpty;
    }

    AST *Parser::arraylist() {
        vector<AST *> elements;
        Token token = this->currentToken;

        eat(LBRACE);

        if (this->currentToken.getType() == COLON) {
            eat(COLON);
            Type Ty = eatType();
            eat(COLON);
            uint32_t num = this->currentToken.getValue().any_cast<int>();
            eat(INTEGER);
            eat(RBRACE);
            return new ArrayInit(Ty, num, token);
        }

        while (this->currentToken.getType() != RBRACE) {
            elements.push_back(expr());

            if (this->currentToken.getType() != RBRACE) {
                eat(COMMA);
            }
        }
        eat(RBRACE);

        return new ArrayInit(elements, elements.size(), token);
    }

    AST *Parser::assignment() {
        AST *left = variable();
        Token token = this->currentToken;
        eat(EQUAL);
        AST *right = checkedExpr();
        return new Assign(token, left, right);
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

        eat(LPAR);
        Param *paramList = (Param *) param();
        // check types
        for (Variable *i: paramList->paramList) {
            if (i->Ty.first == nullptr) {
                throw ExceptionFactory(
                        __SyntaxException,
                        "missing type of member '" + i->id + "'",
                        i->getToken().line, i->getToken().column
                );
            }
        }
        eat(RPAR);

        Type retTy = Type(llvm::Type::getVoidTy(*the_context), "void");

        if (this->currentToken.getType() == TO) {
            eat(TO);
            retTy = eatType();
        }

        if (this->currentToken.getType() == LBRACE) {
            eat(LBRACE);
            AST *compound = statementList();
            eat(RBRACE);
            return new FunctionDecl(id, retTy, paramList, compound, token);
        } else {
            return new FunctionDecl(id, retTy, paramList, nullptr, token);
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

        if (struct_types.find(id) != struct_types.end()) {
            StructInit *struct_init_fun = new StructInit(id, paramList, token);
            return struct_init_fun;
        }

        FunctionCall *fun = new FunctionCall(id, paramList, token);
        return fun;
    }

    AST *Parser::global() {
        Token token = this->currentToken;
        eat(GLOBAL);

        AST *var = variable();

        return new Global(var, token);
    }

    AST *Parser::object() {
        string id;
        Token token = this->currentToken;
        Token last_token = this->lastToken;

        eat(OBJ);
        id = this->currentToken.getValue().any_cast<string>();
        eat(ID);
        eat(LBRACE);

        Param *members_list = (Param *) param();
        vector<Variable *> &li = members_list->paramList;
        vector<llvm::Type *> member_types;
        map<string, int> member_index;
        int index = 0;
        uint32_t struct_size = 0;

        // check types
        // map members' name to sequence
        for (Variable *i: li) {
            if (i->Ty.first == nullptr) {
                throw ExceptionFactory(
                        __SyntaxException,
                        "missing type of member '" + i->id + "'",
                        i->getToken().line, i->getToken().column
                );
            }

            if (i->Ty.second == id) {
                throw ExceptionFactory(
                        __MissingException,
                        "incomplete type '" + i->Ty.second + "'",
                        i->getToken().line, i->getToken().column
                );
            }

            if (i->Ty.second != "real" && i->Ty.second != "vec") {
                if (struct_types.find(i->id) == struct_types.end()) {
                    throw ExceptionFactory(
                            __MissingException,
                            "missing type '" + i->Ty.second + "'",
                            i->getToken().line, i->getToken().column
                    );
                }
            }

            // turn array to pointer
            member_types.push_back(i->Ty.first->isArrayTy() ? i->Ty.first->getPointerTo() : i->Ty.first);
            struct_size += type_size[member_types.back()];
            member_index[i->id] = index++;
        }

        eat(RBRACE);

        // register a struct type
        llvm::StructType *Ty = llvm::StructType::create(*the_context, member_types, NAME_MANGLING(id));

        StructDef *sd = new StructDef(Ty);
        sd->members = member_index;
        struct_types[id] = sd;

        // generate type name
        string struct_type_name = id + "{";
        bool first_flag = true;
        for (auto i: member_types) {
            if (!first_flag) {
                struct_type_name += ",";
            }
            first_flag = false;
            struct_type_name += type_name[i];
        }
        struct_type_name += "}";
        type_name[Ty] = struct_type_name;
        type_size[Ty] = struct_size;

        return new Object(token, id, members_list->paramList);
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
            Variable *var = new Variable(this->currentToken);
            string id = this->currentToken.getValue().any_cast<string>();
            if (paramSet.find(id) != paramSet.end()) {
                string msg =
                        "duplicate variable '" + id + "' in parameters";
                throw ExceptionFactory(__SyntaxException, msg,
                                       this->currentToken.line,
                                       this->currentToken.column);
            }
            paramSet.insert(id);
            var->id = id;
            eat(ID);

            // type is offered, or the first member of Type will be nullptr
            if (this->currentToken.getType() == COLON) {
                eat(COLON);
                Type Ty = eatType();
                var->Ty = Ty;
            }

            param->paramList.push_back(var);
            if (this->currentToken.getType() == COMMA) {
                eat(COMMA);
            } else if (this->currentToken.getType() != ID) {
                return param;
            }
        }
        if (this->currentToken.getType() != ID) {
            return param;
        }
        throw ExceptionFactory(
                __SyntaxException, "unexpected symbol in parameter list",
                this->currentToken.line, this->currentToken.column);
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

    AST *Parser::checkedExpr() {
        if (this->currentToken.getType() == LBRACE) return arraylist();
        if (this->currentToken.getType() == STRING) {
            Token token = this->currentToken;
            eat(STRING);
            return new class String(token);
        }

        return expr();
    }

    /**
     * @description:    return an Integer factor
     * @param:          none
     * @return:         factor
     * @grammar:        Integer | LPAREN
     */
    AST *Parser::factor(void) {
        Token token = this->currentToken;

        if (token.getType() == SIZEOF) {
            return sizeOf();
        }
        if (token.getType() == INTEGER || token.getType() == FLOAT || token.getType() == CHAR) {
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

    AST *Parser::sizeOf() {
        Token token = this->currentToken;

        eat(SIZEOF);
        eat(LPAR);
        if (this->currentToken.getType() == TYPENAME) {
            eat(TYPENAME);
            Type Ty = eatType();
            eat(RPAR);
            return new Sizeof(token, Ty);
        } else {
            AST *var = variable();
            eat(RPAR);
            return new Sizeof(token, var);
        }
    }

    AST *Parser::variable() {
        if (this->currentToken.getType() == DOLLAR)
            eat(DOLLAR);

        Token var = this->currentToken;
        eat(ID);

        /*
          the void type only represent the initial type in source code
          and show to code generator. the variable type in right value
          will be ignored.
        */
        Type Ty = Type(llvm::Type::getVoidTy(*the_context), "void");
        if (this->currentToken.getType() == COLON) {
            eat(COLON);
            Ty = eatType();
            if (Ty.first->isArrayTy()) Ty.first = Ty.first->getPointerTo();
        }

        // process [expr] and .ID
        vector<pair<Variable::offsetType, AST *>> offset;
        TokenType current_type = this->currentToken.getType();
        while (current_type == LSQB || current_type == DOT) {
            if (current_type == LSQB) {
                eat(LSQB);
                AST *val = expr();
                eat(RSQB);
                offset.push_back(pair<Variable::offsetType, AST *>(Variable::ARRAY, val));
            } else {
                eat(DOT);
                Token id = this->currentToken;
                eat(ID);
                offset.push_back(pair<Variable::offsetType, AST *>(Variable::MEMBER, new Variable(id)));
            }
            current_type = this->currentToken.getType();
        }


        return new Variable(var, Ty, offset);
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

    Type Parser::eatType() {
        if (this->currentToken.getType() == REAL) {
            eat(REAL);
            return Type(llvm::Type::getDoubleTy(*the_context), "real");
        } else if (this->currentToken.getType() == CHAR) {
            eat(CHAR);
            return Type(llvm::Type::getInt8Ty(*the_context), "char");
        } else if (this->currentToken.getType() == VEC) {
            eat(VEC);
            eat(LSQB);
            if (this->currentToken.getType() != RSQB) {
                // Type can be any types, even another vector
                Type nest = eatType();
                eat(SEMI);
                int array_size = 0;
                if (this->currentToken.getType() == INTEGER) {
                    array_size = this->currentToken.getValue().any_cast<int>();
                    eat(INTEGER);
                } else {
                    throw ExceptionFactory(
                            __SyntaxException,
                            "array size must be provided",
                            this->currentToken.line, this->currentToken.column
                    );
                }
                eat(RSQB);
                if (array_size != 0) {
                    llvm::Type *Ty = llvm::ArrayType::get(nest.first, array_size);
                    type_name[Ty] = "vec[" + type_name[nest.first] + ";" + to_string(array_size) + "]";
                    type_name[Ty->getPointerTo()] = type_name[Ty] + "*";
                    type_size[Ty] = type_size[nest.first] * array_size;
                    return Type(Ty, "vec");
                } else {
                    llvm::Type *Ty = nest.first->getPointerTo();
                    type_name[Ty] = type_name[nest.first] + "*";
                    type_name[Ty->getPointerTo()] = type_name[Ty] + "*";
                    type_size[Ty] = sizeof(size_t *);
                    return Type(Ty, "vec");
                }
            }
            throw ExceptionFactory(
                    __SyntaxException,
                    "array type and size must be provided",
                    this->currentToken.line, this->currentToken.column
            );
        } else if (this->currentToken.getType() == ID) {
            string id = this->currentToken.getValue().any_cast<string>();
            if (struct_types.find(id) == struct_types.end()) {
                throw ExceptionFactory(
                        __MissingException,
                        "missing type '" + id + "'",
                        this->currentToken.line, this->currentToken.column
                );
            }

            Type Ty = Type(struct_types[id]->Ty, this->currentToken.getValue().any_cast<string>());
            eat(ID);
            return Ty;
        } else {
            throw ExceptionFactory(
                    __SyntaxException,
                    "type is unrecognized",
                    this->currentToken.line, this->currentToken.column
            );
        }
    }
} // namespace AVSI