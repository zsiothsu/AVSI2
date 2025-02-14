/*
 * Parser.cpp 2022
 *
 * include Parser class
 *
 * LLVM IR code generator
 *
 * MIT License
 *
 * Copyright (c) 2022 Chipen Hsiao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../inc/Parser.h"
#include <filesystem>

uint16_t err_count = 0;
uint16_t warn_count = 0;

#undef __AVSI_DEBUG__TOKEN_NAME
#undef __AVSI_DEBUG__PARSER

#ifdef __AVSI_DEBUG__PARSER
#define PARSE_LOG(NESYMBOL) _PARSE_LOG(NESYMBOL)
#else
#define PARSE_LOG(NESYMBOL)
#endif

#define _PARSE_LOG(NESYMBOL) \
do { \
    clog << "parsing M[" \
         << #NESYMBOL \
         << "," \
         << token_name[this->currentToken.getType()] \
         << "]" \
         << endl; \
} while(0)

extern bool opt_warning;

namespace AVSI {
    /*******************************************************
     *                     variable                        *
     *******************************************************/
    extern llvm::LLVMContext *the_context;
    extern llvm::Module *the_module;
    extern string module_name;
    extern string module_name_nopath;
    extern vector<string> module_path;
    extern vector<string> module_path_with_module_name;

    extern map<string, StructDef *> struct_types;

    extern map<llvm::Type *, string> type_name;
    extern map<llvm::Type *, uint32_t> type_size;

    extern map<string, string> module_name_alias;

    extern llvm::Type *F64_TY;
    extern llvm::Type *F32_TY;
    extern llvm::Type *I128_TY;
    extern llvm::Type *I64_TY;
    extern llvm::Type *I32_TY;
    extern llvm::Type *I16_TY;
    extern llvm::Type *I8_TY;
    extern llvm::Type *I1_TY;
    extern llvm::Type *VOID_TY;

    extern map<llvm::Type *, string> type_name;

    extern set<llvm::Type *> simple_types;
    extern map<TokenType, llvm::Type *> token_to_simple_types;

    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    Parser::Parser(void) {}

    Parser::Parser(Lexer *lexer) {
        this->lexer = lexer;
        this->currentToken = lexer->getNextToken();
        this->lastToken = this->currentToken;

#ifdef __AVSI_DEBUG__TOKEN_NAME
        cout << '<'
             << token_name[this->currentToken.getType()]
             << ','
             << string(this->currentToken.getValue())
             << '>'
             << endl;
#endif
    }

    Parser::~Parser() {}

    /*******************************************************
     *                         parser                      *
     *******************************************************/
    pair<map<string, StructDef *>::iterator, string> find_struct(vector<string> modinfo, string &name) {
        string id = name;
        // try no mangle
        auto ty = struct_types.find(id);

        if (ty == struct_types.end()) {
            if (modinfo.empty()) {
                // try local
                id.clear();
                for (auto i: module_path_with_module_name) {
                    id.append(i + "::");
                }
                id.append(name);
                ty = struct_types.find(id);
            } else if (modinfo[0] == "root") {
                // try external, absolute path
                id.clear();
                modinfo.erase(modinfo.begin());

                vector<string> fullpath;
                fullpath.insert(fullpath.end(), package_path.begin(), package_path.end());
                fullpath.insert(fullpath.end(), modinfo.begin(), modinfo.end());
                for (auto i: fullpath) {
                    id.append(i + "::");
                }
                id.append(name);
                ty = struct_types.find(id);
            } else {
                // try alias
                if (ty == struct_types.end()) {
                    string head = modinfo[0];
                    if (module_name_alias.find(head) != module_name_alias.end()) {
                        auto path_cut = modinfo;
                        path_cut.erase(path_cut.begin());
                        head = module_name_alias[head];
                        auto head_to_origin = getpathUnresolvedToList(head);
                        for (auto i: path_cut) {
                            head_to_origin.push_back(i);
                        }

                        id.clear();
                        for (auto i: head_to_origin) {
                            id.append(i + "::");
                        }
                        id.append(name);
                        ty = struct_types.find(id);
                    }
                }

                // try external, relative path
                if (ty == struct_types.end()) {
                    id.clear();
                    for (auto i: module_path) {
                        id.append(i + "::");
                    }
                    for (auto i: modinfo) {
                        id.append(i + "::");
                    }
                    id.append(name);

                    ty = struct_types.find(id);
                }
            }
        }

        return pair<map<string, StructDef *>::iterator, string>(ty, id);
    }


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

            try {
                this->currentToken = this->lexer->getNextToken();
            } catch (Exception &e) {
                throw e;
            }

#ifdef __AVSI_DEBUG__TOKEN_NAME
            cout << '<'
                 << token_name[this->currentToken.getType()]
                 << ','
                 << string(this->currentToken.getValue())
                 << '>'
                 << endl;
#endif

#ifdef __AVSI_DEBUG__PARSER
            clog << "get " << token_name[this->lastToken.getType()] << endl;
#endif
        } else {
            throw ExceptionFactory<SyntaxException>(
                    "invalid syntax, expected " + token_name[type],
                    this->currentToken.line,
                    this->currentToken.column
            );
        }
    }

    shared_ptr<AST> Parser::program() {
        try {
            return statementList();
        } catch (Exception &e) {
            throw e;
        }
    }

    shared_ptr<AST> Parser::statementList() {
        PARSE_LOG(COMPOUND);

        shared_ptr<Compound> root = make_shared<Compound>(Compound(this->lastToken));
        shared_ptr<AST> node;
        while (this->currentToken.getType() != END) {
            try {
                node = statement();
                if (node == ASTEmpty) break;
                root->child.push_back(node);
            } catch (Exception e) {
                if (e.type() != __ErrReport) {
                    std::cerr << __COLOR_RED
                              << input_file_name
                              << ":" << e.line << ":" << e.column + 1 << ": "
                              << e.what()
                              << __COLOR_RESET << std::endl;
                }

                auto tokenIsNotAStart = [](Token t) -> bool {
                    for (TokenType i: FIRST_STATEMENT) {
                        if (i == t.getType()) return false;
                    }
                    return true;
                };

                while (tokenIsNotAStart(this->currentToken) && this->currentToken.getType() != END) {
                    eat(this->currentToken.getType());
                }
                if (this->currentToken.getType() == END) goto err;
            }
        }

        return root;

        err:
        throw ExceptionFactory<ErrReport>("", 0, 0);
    }

    shared_ptr<AST> Parser::statement() {
        bool is_export = true;
        bool is_mangle = true;
        bool is_inline = false;
        bool is_always_inline = false;
        bool is_noinline = false;
        bool is_pure = false;

        auto token_is_function_attr = [](Token t) -> bool {
            for (TokenType i: FUNCTION_ATTR) {
                if (i == t.getType()) return true;
            }
            return false;
        };

        while (token_is_function_attr(this->currentToken)) {
            if (this->currentToken.getType() == PUBLIC) {
                is_export = true;
                eat(PUBLIC);
            }

            if (this->currentToken.getType() == PRIVATE) {
                is_export = false;
                eat(PRIVATE);
            }

            if (this->currentToken.getType() == NOMANGLE) {
                is_mangle = false;
                eat(NOMANGLE);
            }

            if (this->currentToken.getType() == INLINE) {
                is_inline = true;
                eat(INLINE);
            }

            if (this->currentToken.getType() == ALWAYS_INLINE) {
                is_always_inline = true;
                eat(ALWAYS_INLINE);
            }

            if (this->currentToken.getType() == NOINLINE) {
                is_noinline = true;
                eat(NOINLINE);
            }

            if (this->currentToken.getType() == PURE) {
                is_pure = true;
                eat(PURE);
            }
        }

        TokenType token_type = this->currentToken.getType();

        if (token_type == FUNCTION) {
            PARSE_LOG(STATEMENT);
            shared_ptr<FunctionDecl> function = static_pointer_cast<FunctionDecl>(functionDecl());
            function->is_export = is_export;
            function->is_mangle = is_mangle;
            function->is_inline = is_inline;
            function->is_always_inline = is_always_inline;
            function->is_noinline = is_noinline;
            function->is_pure = is_pure;
            return function;
        } else if (token_type == RETURN) {
            PARSE_LOG(STATEMENT);
            return returnExpr();
        } else if (token_type == ID) {
            PARSE_LOG(STATEMENT);
            return IDHead();
        } else if (token_type == IF) {
            PARSE_LOG(STATEMENT);
            return IfStatement();
        } else if (token_type == FOR) {
            PARSE_LOG(STATEMENT);
            return forStatement();
        } else if (token_type == WHILE) {
            PARSE_LOG(STATEMENT);
            return WhileStatement();
        } else if (token_type == GENERIC) {
            PARSE_LOG(STATEMENT);
            shared_ptr<Generic> gen = static_pointer_cast<Generic>(generic());
            gen->is_mangle = is_mangle;
            return gen;
        } else if (token_type == GLOBAL) {
            PARSE_LOG(STATEMENT);
            shared_ptr<Global> glb = static_pointer_cast<Global>(global());
            glb->is_export = is_export;
            glb->is_mangle = is_mangle;
            return glb;
        } else if (token_type == OBJ) {
            PARSE_LOG(STATEMENT);
            shared_ptr<Object> obj = static_pointer_cast<Object>(object(is_mangle));
            obj->is_export = is_export;
            obj->is_mangle = is_mangle;
            return obj;
        } else if (token_type == BREAK || token_type == CONTINUE) {
            PARSE_LOG(LOOPCTRL);
            return loopCtrl();
        } else if (token_type == MODULE) {
            PARSE_LOG(STATEMENT);
            return moduleDef();
        } else if (token_type == IMPORT) {
            PARSE_LOG(STATEMENT);
            return moduleImport();
        } else if (token_type == GRAD) {
            PARSE_LOG(STATEMENT);
            return grad();
        }

        for (auto op: ExprOp) {
            if (token_type == op) {
                Token token = this->currentToken;
                shared_ptr<AST> ast = checkedExpr();
                return make_shared<BlockExpr>(BlockExpr(token, ast));
            }
        }

        return ASTEmpty;
    }

    shared_ptr<AST> Parser::arraylist() {
        PARSE_LOG(ARRAY);

        vector<shared_ptr<AST>> elements;
        Token token = this->currentToken;

        eat(LBRACE);

        bool is_vec = false;
        if (this->currentToken.getType() == VEC) {
            is_vec = true;
            eat(VEC);
        } else if (this->currentToken.getType() == ARR) {
            eat(ARR);
        } 

        if (this->currentToken.getType() == COLON) {
            eat(COLON);
            Type Ty = eatType();
            eat(COLON);
            uint32_t num = this->currentToken.getValue().any_cast<int>();
            eat(INTEGER);
            eat(RBRACE);
            return make_shared<ArrayInit>(ArrayInit(Ty, num, is_vec, token));
        }

        while (this->currentToken.getType() != RBRACE) {
            elements.push_back(expr());

            if (this->currentToken.getType() != RBRACE) {
                eat(COMMA);
            }
        }
        eat(RBRACE);

        return make_shared<ArrayInit>(ArrayInit(elements, elements.size(), is_vec, token));
    }

    shared_ptr<AST> Parser::assignment() {
        PARSE_LOG(ASSIGNMENT);

        shared_ptr<AST> left = variable();
        Token token = this->currentToken;
        eat(EQUAL);
        shared_ptr<AST> right = checkedExpr();
        return make_shared<Assign>(Assign(token, left, right));
    }

    shared_ptr<AST> Parser::forStatement() {
        PARSE_LOG(FORSTATEMENT);

        Token token = this->currentToken;

        shared_ptr<AST> initList = nullptr;
        shared_ptr<AST> condition = nullptr;
        shared_ptr<AST> adjustment = nullptr;
        shared_ptr<AST> compound = nullptr;
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

        return make_shared<For>(For(initList, condition, adjustment, compound, noCondition, token));
    }

    shared_ptr<AST> Parser::functionDecl() {
        PARSE_LOG(FUNCTIONDECL);

        Token token = this->currentToken;
        eat(FUNCTION);
        string id = this->currentToken.getValue().any_cast<string>();
        auto struct_info = this->currentToken.getModInfo();
        token.setModInfo(struct_info);
        eat(ID);

        eat(LPAR);
        shared_ptr<Param> paramList = static_pointer_cast<Param>(param());
        // check types
        for (Variable *i: paramList->paramList) {
            if (i->Ty.first == nullptr) {
                throw ExceptionFactory<SyntaxException>(
                        "missing type of member '" + i->id + "'",
                        i->getToken().line, i->getToken().column
                );
            }
        }
        eat(RPAR);

        Type retTy = Type(llvm::Type::getVoidTy(*the_context), "void");

        if (this->currentToken.getType() == TO) {
            PARSE_LOG(FUNCTIONTYPE);

            eat(TO);
            retTy = eatType();
        }

        if (this->currentToken.getType() == LBRACE) {
            PARSE_LOG(FUNCTIONBODY);

            eat(LBRACE);
            shared_ptr<AST> compound = statementList();
            eat(RBRACE);
            return make_shared<FunctionDecl>(FunctionDecl(id, retTy, paramList, compound, token));
        } else {
            return make_shared<FunctionDecl>(FunctionDecl(id, retTy, paramList, nullptr, token));
        }
    }

    shared_ptr<AST> Parser::functionCall() {
        PARSE_LOG(FUNCTIONCALL);

        Token token = this->currentToken;
        string id = this->currentToken.getValue().any_cast<string>();
        string id_clone = id;
        eat(ID);

        vector<shared_ptr<AST> > paramList;
        eat(LPAR);
        if (this->currentToken.getType() != RPAR) {
            paramList.push_back(checkedExpr());
            while (this->currentToken.getType() == COMMA) {
                eat(COMMA);
                paramList.push_back(checkedExpr());
            }
        }
        eat(RPAR);

        auto ty = find_struct(token.getModInfo(), id_clone);

        if (ty.first != struct_types.end()) {
            shared_ptr<StructInit> struct_init_fun = make_shared<StructInit>(StructInit(ty.second, paramList, token));
            return struct_init_fun;
        }

        shared_ptr<FunctionCall> fun = make_shared<FunctionCall>(FunctionCall(id, paramList, token));
        return fun;
    }

    shared_ptr<AST> Parser::generic() {
        PARSE_LOG(GENERIC);

        Token token = this->currentToken;
        eat(GENERIC);

        string id = this->currentToken.getValue().any_cast<string>();
        auto struct_info = this->currentToken.getModInfo();
        token.setModInfo(struct_info);

        eat(ID);

        int idx = 0;
        if (currentToken.getType() == LT) {
            eat(LT);
            idx = this->currentToken.getValue().any_cast<int>();
            eat(INTEGER);
            eat(GT);
        }

        eat(LBRACE);

        vector<pair<string,Type>> func_list;
        string default_func;
        while(this->currentToken.getType() != RBRACE) {
            Token token = this->currentToken;
            auto ty = eatType();
            eat(COLON);
            string id;
            if (this->currentToken.getType() == ID) {
                id = this->currentToken.getValue().any_cast<string>();
            }
            eat(ID);

            if (ty.second == "default") {
                default_func = id;
                break;
            } else {
                // check types
                if (ty.first == nullptr) {
                    throw ExceptionFactory<SyntaxException>(
                        "missing type of function '" + id + "'",
                        token.line, token.column
                    );
                }
                func_list.push_back({id, ty});
            }

            if (this->currentToken.getType() == RBRACE) {
                break;
            } else {
                eat(COMMA);
            }
        }

        eat(RBRACE);

        return make_shared<Generic>(Generic(id, func_list, default_func, idx, token));
    }

    shared_ptr<AST> Parser::global() {
        PARSE_LOG(GLOBAL);

        Token token = this->currentToken;
        eat(GLOBAL);

        shared_ptr<AST> var = variable();

        return make_shared<Global>(Global(var, token));
    }

    shared_ptr<AST> Parser::grad() {
        PARSE_LOG(GRAD);

        Token token = this->currentToken;
        eat(GRAD);
        eat(LPAR);
        auto expr_to_grad = expr();

        eat(COMMA);

        vector<pair<string, int>> vars;

        while (
            (this->currentToken.getType() == ID)
            || (this->currentToken.getType() == LBRACE)
        ) {
            if (this->currentToken.getType() == ID) {
                auto var = variable();
                if (!static_pointer_cast<Variable>(var)->offset.empty()) {
                    throw ExceptionFactory<LogicException>(
                            "the derivative variable cannot be a member function, array element, or function",
                            var->getToken().line, var->getToken().column
                    );
                }
                vars.push_back({static_pointer_cast<Variable>(var)->id, 1});
            } else {
                eat(LBRACE);
                auto var = variable();
                eat(COMMA);
                Token num_token = this->currentToken;
                eat(INTEGER);
                eat(RBRACE);
                vars.push_back({static_pointer_cast<Variable>(var)->id, num_token.getValue().any_cast<int>()});
            }

            if (this->currentToken.getType() == COMMA) {
                eat(COMMA);
            } else {
                break;
            }
        }


        eat(RPAR);


        return make_shared<Grad>(Grad(expr_to_grad, vars, token));
    }

    shared_ptr<AST> Parser::moduleDef() {
        PARSE_LOG(MODULEDEF);

        static bool mod_named = false;

        if (mod_named) {
            Warning(
                    "Module has named before.",
                    this->currentToken.line, this->currentToken.column
            );
        }

        eat(MODULE);
        if (this->currentToken.getType() == ID) {
            vector<string> path;
            vector<string> modinfo = this->currentToken.getModInfo();
            for (auto &p: package_path) {
                path.push_back(p);
            }
            if (!modinfo.empty() && modinfo[0] == "root") {
                modinfo.erase(modinfo.begin());
            }
            path.insert(path.end(), modinfo.begin(), modinfo.end());
            module_path = path;
            module_path_with_module_name = path;

            /**
             * if current file is __init__.sl， the    parent folder should be added to searh path
             * for example:
             *
             * this is a normal file:
             * root/normal.sl:
             *      mod com::avsi::normal
             * search root path will be com::avsi
             *
             * but for __init__.sl:
             * root/foo/__init__.sl:
             *      mod com::avsi::foo
             * search root path will be com::avsi instead of com::avsi::foo.
             *
             * to fix the problem, mod name should be append to search path
             */
            if (input_file_name_no_suffix == MODULE_INIT_NAME) {
                std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
                module_path.push_back(dir);
                module_path_with_module_name.push_back(dir);
            }

            module_name_nopath = this->currentToken.getValue().any_cast<string>();

            if (input_file_name_no_suffix != MODULE_INIT_NAME) {
                module_path_with_module_name.push_back(module_name_nopath);
            }

            path.push_back(module_name_nopath);
            module_name = __getModuleNameByPath(path);
            mod_named = true;
            the_module->setModuleIdentifier(module_name);

            string path_unresolved = getpathListToUnresolved(path);
            module_name_alias[path_unresolved] = path_unresolved;
            module_name_alias[""] = path_unresolved;
        }
        eat(ID);
        return ASTEmptyNotEnd;
    }

    shared_ptr<AST> Parser::moduleImport() {
        PARSE_LOG(MODULEIMPORT);

        eat(IMPORT);
        if (this->currentToken.getType() == ID) {
            Token id = this->currentToken;
            vector<string> path = id.getModInfo();
            eat(ID);
            string as;
            if (this->currentToken.getType() == AS) {
                eat(AS);
                as = this->currentToken.getValue().any_cast<string>();
                eat(ID);
            }

            llvm_import_module(
                    path,
                    id.getValue().any_cast<string>(),
                    id.line,
                    id.column,
                    as);
        }

        return ASTEmptyNotEnd;
    }

    shared_ptr<AST> Parser::object(bool is_mangle) {
        PARSE_LOG(OBJECTDEF);

        string id;
        Token token = this->currentToken;
        Token last_token = this->lastToken;

        eat(OBJ);
        if (is_mangle) {
            for (auto &i: module_path_with_module_name) {
                id.append(i + "::");
            }
        }
        id.append(this->currentToken.getValue().any_cast<string>());
        eat(ID);
        eat(LBRACE);

        shared_ptr<Param> members_list = static_pointer_cast<Param>(param());
        vector<Variable *> &li = members_list->paramList;
        vector<llvm::Type *> member_types;
        map<string, int> member_index;
        int index = 0;
        uint32_t struct_size = 0;

        llvm::NamedMDNode *meta = the_module->getOrInsertNamedMetadata("struct." + id);
        vector<llvm::Metadata *> mdlist;

        // check types
        // map members' name to sequence
        for (Variable *i: li) {
            if (i->Ty.first == nullptr) {
                throw ExceptionFactory<SyntaxException>(
                        "missing type of member '" + i->id + "'",
                        i->getToken().line, i->getToken().column
                );
            }

            if (i->Ty.second == id) {
                throw ExceptionFactory<MissingException>(
                        "incomplete type '" + i->Ty.second + "'",
                        i->getToken().line, i->getToken().column
                );
            }

            if (simple_types.find(i->Ty.first) == simple_types.end() && i->Ty.second != "arr") {
                if (struct_types.find(i->Ty.second) == struct_types.end()) {
                    throw ExceptionFactory<MissingException>(
                            "missing type '" + i->Ty.second + "'",
                            i->getToken().line, i->getToken().column
                    );
                }
            }

            member_types.push_back(i->Ty.first);
            struct_size += i->Ty.first->isPtrOrPtrVectorTy() ? PTR_SIZE : type_size[i->Ty.first];
            member_index[i->id] = index++;
            auto mdnode = llvm::MDNode::get(*the_context, {llvm::MDString::get(*the_context, i->id),
                                                           llvm::MDString::get(*the_context, "struct_member")});
            meta->addOperand(mdnode);
        }

        eat(RBRACE);

        // register a struct type
        llvm::StructType *Ty = llvm::StructType::create(*the_context, member_types, id);

        the_module->getOrInsertGlobal(
                ".reserve.object." + id, Ty,
                [&] {
                    return new llvm::GlobalVariable(
                            *the_module,
                            Ty,
                            false,
                            llvm::GlobalValue::ExternalLinkage,
                            llvm::Constant::getNullValue(Ty),
                            ".reserve.object." + id);
                });


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

        auto meta_size = llvm::MDNode::get(*the_context, {llvm::MDString::get(*the_context, to_string(struct_size)),
                                                          llvm::MDString::get(*the_context, "struct_size")});
        meta->addOperand(meta_size);
        auto meta_name = llvm::MDNode::get(*the_context, {llvm::MDString::get(*the_context, struct_type_name),
                                                          llvm::MDString::get(*the_context, "struct_name")});
        meta->addOperand(meta_name);

        return make_shared<Object>(Object(token, id, members_list->paramList));
    }

    shared_ptr<AST> Parser::IfStatement() {
        Token token = this->currentToken;
        TokenType type = this->currentToken.getType();
        shared_ptr<AST> condition = nullptr;
        bool noCondition = false;

        if (type == FI) {
            PARSE_LOG(IfLINK);
            eat(FI);
            return ASTEmpty;
        }

        if (type == IF) {
            PARSE_LOG(IfSTATEMENT);
            eat(IF);

            bool have_SQB = false;
            if (this->currentToken.getType() == LSQB) {
                eat(LSQB);
                have_SQB = true;
            }

            if ((have_SQB && this->currentToken.getType() == RSQB) || this->currentToken.getType() == THEN) {
                throw ExceptionFactory<LogicException>(
                        "if condition must be privided",
                        this->lastToken.line, this->lastToken.column
                );
            }

            condition = expr();
            if (have_SQB) eat(RSQB);
            eat(THEN);
        } else if (type == ELIF) {
            PARSE_LOG(IfLINK);
            eat(ELIF);
            eat(LSQB);

            if (this->currentToken.getType() == RSQB) {
                throw ExceptionFactory<SyntaxException>(
                        "if condition must be privided",
                        this->lastToken.line, this->lastToken.column
                );
            }

            condition = expr();
            eat(RSQB);
            eat(THEN);
        } else {
            PARSE_LOG(IfLINK);
            eat(ELSE);
            noCondition = true;
        }

        shared_ptr<AST> compound = statementList();
        shared_ptr<AST> next = IfStatement();

        return make_shared<If>(If(condition, noCondition, compound, next, token));
    }

    shared_ptr<AST> Parser::param() {
        PARSE_LOG(PARAM);

        shared_ptr<Param> param = make_shared<Param>(Param(false, this->lastToken));
        set<string> paramSet;
        while ((this->currentToken.getType() == ID) || (this->currentToken.getType() == VARARG)) {
            if (this->currentToken.getType() == VARARG) {
                eat(VARARG);
                param->is_va_arg = true;
                return param;
            }

            Variable *var = new Variable(this->currentToken);
            string id = this->currentToken.getValue().any_cast<string>();
            if (paramSet.find(id) != paramSet.end()) {
                string msg =
                        "duplicate variable '" + id + "' in parameters";
                throw ExceptionFactory<SyntaxException>(
                        msg,
                        this->currentToken.line,
                        this->currentToken.column
                );
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
        throw ExceptionFactory<SyntaxException>(
                "unexpected symbol in parameter list",
                this->currentToken.line,
                this->currentToken.column
        );
        return param;
    }

    shared_ptr<AST> Parser::expr() {
        return logic_or_expr();
    }

    shared_ptr<AST> Parser::logic_or_expr() {
        PARSE_LOG(OREXPR);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = logic_and_expr();
        auto type = this->currentToken.getType();
        while (type == OR) {
            Token opt = this->currentToken;
            eat(OR);
            res = make_shared<BinOp>(BinOp(res, opt, logic_and_expr()));
            type = this->currentToken.getType();
        }

        return res;
    }

    shared_ptr<AST> Parser::logic_and_expr() {
        PARSE_LOG(ANDEXPR);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = bit_or_expr();
        auto type = this->currentToken.getType();
        while (type == AND) {
            Token opt = this->currentToken;
            eat(AND);
            res = make_shared<BinOp>(BinOp(res, opt, bit_or_expr()));
            type = this->currentToken.getType();
        }

        return res;
    }

    shared_ptr<AST> Parser::bit_or_expr() {
        PARSE_LOG(BITOREXPR);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = bit_and_expr();
        auto type = this->currentToken.getType();
        while (type == BITOR) {
            Token opt = this->currentToken;
            eat(BITOR);
            res = make_shared<BinOp>(BinOp(res, opt, bit_and_expr()));
            type = this->currentToken.getType();
        }

        return res;
    }

    shared_ptr<AST> Parser::bit_and_expr() {
        PARSE_LOG(BITANDEXPR);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = equivalence_expr();
        auto type = this->currentToken.getType();
        while (type == BITAND) {
            Token opt = this->currentToken;
            eat(BITAND);
            res = make_shared<BinOp>(BinOp(res, opt, equivalence_expr()));
            type = this->currentToken.getType();
        }

        return res;
    }

    shared_ptr<AST> Parser::equivalence_expr() {
        PARSE_LOG(EQEXPR);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = compare_expr();
        auto type = this->currentToken.getType();
        while (type == EQ || type == NE) {
            Token opt = this->currentToken;
            eat(type);
            res = make_shared<BinOp>(BinOp(res, opt, compare_expr()));
            type = this->currentToken.getType();
        }

        return res;
    }

    shared_ptr<AST> Parser::compare_expr() {
        PARSE_LOG(CMPEXPR);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = shift_expr();
        auto type = this->currentToken.getType();
        while (type == GT || type == LT || type == GE || type == LE) {
            Token opt = this->currentToken;
            eat(type);
            res = make_shared<BinOp>(BinOp(res, opt, shift_expr()));
            type = this->currentToken.getType();
        }

        return res;
    }

    shared_ptr<AST> Parser::shift_expr() {
        PARSE_LOG(SHIFYEXPR);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = basic_expr();
        auto type = this->currentToken.getType();
        while (type == SHR || type == SHRU || type == SHL) {
            Token opt = this->currentToken;
            eat(type);
            res = make_shared<BinOp>(BinOp(res, opt, basic_expr()));
            type = this->currentToken.getType();
        }

        return res;
    }

    /**
     * @description:    arithmetic expression parser / interpreter.
     * @param:          None
     * @return:         result of arithmetic expression
     * @throw:          SyntaxException
     * @grammar:        expr: term ((ADD | DEC) term)*
     *                  term: Integer;
     */
    shared_ptr<AST> Parser::basic_expr(void) {
        PARSE_LOG(EXPR);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = term();
        auto type = this->currentToken.getType();
        while (type == PLUS || type == MINUS) {
            Token opt = this->currentToken;
            eat(type);
            res = make_shared<BinOp>(BinOp(res, opt, term()));
            type = this->currentToken.getType();
        }

        return res;
    }

    shared_ptr<AST> Parser::checkedExpr() {
        PARSE_LOG(EXPRCHECK);

        if (this->currentToken.getType() == LBRACE) return arraylist();
        if (this->currentToken.getType() == STRING) {
            Token token = this->currentToken;
            eat(STRING);

            string buffer = token.getValue().any_cast<string>();
            while (this->currentToken.getType() == STRING) {
                Token next = this->currentToken;
                buffer.append(next.getValue().any_cast<string>());
                eat(STRING);
            }

            token = Token(STRING, buffer, token.line, token.column);

            return shared_ptr<AST>(new class String(token));
        }

        return expr();
    }

    /**
     * @description:    return an Integer factor
     * @param:          none
     * @return:         factor
     * @grammar:        Integer | LPAREN
     */
    shared_ptr<AST> Parser::factor(void) {
        PARSE_LOG(FACTOR);

        Token token = this->currentToken;
        TokenType ty = token.getType();

        shared_ptr<AST> ret;

        switch (ty) {
            case SIZEOF:
                ret = sizeOf();
                break;
            case GRAD:
                ret = grad();
                break;
            case INTEGER:
            case FLOAT:
            case CHAR:
                eat(ty);
                ret = make_shared<Num>(Num(token));
                break;
            case TRUE:
            case FALSE:
                eat(ty);
                ret = make_shared<Boolean>(Boolean(token));
                break;
            case PLUS:
            case MINUS:
            case NOT:
            case BITCPL:
            case BITAND:
            case STAR:
                eat(ty);
                ret = make_shared<UnaryOp>(UnaryOp(token, factor()));
                break;
            case ID:
                if (this->lexer->peekNextToken().getType() == LPAR) {
                    ret = functionCall();
                } else {
                    ret = variable();
                }
                break;
            case DOLLAR:
                ret = variable();
                break;
            case LPAR:
                eat(LPAR);
                ret = expr();
                eat(RPAR);
                break;
            case IF:
                ret = IfStatement();
                break;
            default:
                break;
        }

        if (ret.get()) {
            if (this->currentToken.getType() == AS) {
                Token as = this->currentToken;
                eat(AS);
                Type Ty = eatType();
                ret = make_shared<TypeTrans>(TypeTrans(ret, Ty, as));
            }
            return ret;
        }

        if (ty == RPAR) {
            if (this->parenCnt <= 0)
                throw ExceptionFactory<SyntaxException>(
                        "unmatched ')'",
                        token.line, token.column
                );
            return make_shared<NoneAST>(NoneAST());
        } else
            throw ExceptionFactory<SyntaxException>(
                    "unrecognized factor in expression",
                    token.line, token.column
            );
    }

    /**
     * @description:    parser entry
     * @param:          None
     * @return:         root of parse result.
     */
    shared_ptr<AST> Parser::parse(void) { return program(); }

    shared_ptr<AST> Parser::returnExpr(void) {
        PARSE_LOG(RETURNEXPR);

        Token token = this->currentToken;
        eat(RETURN);

        shared_ptr<AST> ret = nullptr;
        if (this->currentToken.isExpr()) ret = checkedExpr();

        return make_shared<Return>(Return(token, ret));
    }

    /**
     * @description:    return MUL / DIV result
     * @param:          None
     * @return:         term for expr
     * @throw           SyntaxException
     * @c:        term: factor ((MUL | DIV) factor)*
     *                  factor: Integer
     */
    shared_ptr<AST> Parser::term(void) {
        PARSE_LOG(TERM);

        if (this->currentToken.getType() == SEMI) return ASTEmpty;

        shared_ptr<AST> res = factor();
        auto type = this->currentToken.getType();
        while (type == STAR || type == SLASH || type == REM) {
            Token opt = this->currentToken;
            eat(type);
            res = make_shared<BinOp>(BinOp(res, opt, factor()));
            type = this->currentToken.getType();
        }

        return res;
    }

    shared_ptr<AST> Parser::sizeOf() {
        PARSE_LOG(SIZEOF);

        Token token = this->currentToken;

        eat(SIZEOF);
        eat(LPAR);
        if (this->currentToken.getType() == TYPENAME) {
            eat(TYPENAME);
            Type Ty = eatType();
            eat(RPAR);
            return make_shared<Sizeof>(Sizeof(token, Ty));
        } else {
            shared_ptr<AST> var = variable();
            eat(RPAR);
            return make_shared<Sizeof>(Sizeof(token, var));
        }
    }

    shared_ptr<AST> Parser::variable() {
        PARSE_LOG(VARIABLE);

        if (this->currentToken.getType() == DOLLAR)
            eat(DOLLAR);

        Token var = this->currentToken;
        eat(ID);

        /*
          the void type only represent the initial type in source code
          and show to code generator. the variable type in right value
          will be ignored.
        */
        Type Ty = Type(llvm::Type::getVoidTy(*the_context), "none");
        if (this->currentToken.getType() == COLON) {
            eat(COLON);
            Ty = eatType();
        }

        // process [expr] and .ID
        vector<pair<Variable::offsetType, shared_ptr<AST> >> offset;

        TokenType ty = this->currentToken.getType();
        Token token;
        shared_ptr<AST> right;
        Variable::offsetType offset_ty;
        while (ty == DOT || ty == LSQB) {
            while (ty == DOT) {
                token = this->currentToken;
                eat(DOT);

                ty = this->currentToken.getType();
                if (ty == ID) {
                    if (this->lexer->peekNextToken().getType() == LPAR) {
                        right = functionCall();
                        offset_ty = Variable::offsetType::FUNCTION;
                    } else {
                        right = make_shared<Variable>(Variable(this->currentToken));
                        eat(this->currentToken.getType());
                        offset_ty = Variable::offsetType::MEMBER;
                    }
                } else if (ty == INTEGER) {
                    right = make_shared<Num>(Num(this->currentToken));
                    eat(INTEGER);
                }

                offset.push_back({offset_ty, right});
                ty = this->currentToken.getType();
            }

            while (ty == LSQB) {
                token = this->currentToken;
                eat(LSQB);
                right = expr();
                eat(RSQB);

                offset.push_back({Variable::offsetType::ARRAY, right});
                ty = this->currentToken.getType();
            }
        }


        return make_shared<Variable>(Variable(var, Ty, offset));
    }

    shared_ptr<AST> Parser::WhileStatement() {
        PARSE_LOG(WHILESTATEMENT);

        Token token = this->currentToken;
        shared_ptr<AST> condition;
        shared_ptr<AST> compound;

        eat(WHILE);

        bool have_SQB = false;
        if (this->currentToken.getType() == LSQB) {
            eat(LSQB);
            have_SQB = true;
        }

        if ((have_SQB && this->currentToken.getType() == RSQB) || this->currentToken.getType() == DO) {
            throw ExceptionFactory<LogicException>(
                    "while condition must be privided",
                    this->lastToken.line,
                    this->lastToken.column
            );
        }

        condition = expr();
        if (have_SQB) eat(RSQB);
        eat(DO);
        compound = statementList();
        eat(DONE);

        return make_shared<While>(While(condition, compound, token));
    }

    shared_ptr<AST> Parser::loopCtrl() {
        if (this->currentToken.getType() == BREAK) {
            eat(BREAK);
            return make_shared<LoopCtrl>(LoopCtrl(LoopCtrl::LoopCtrlType::CTRL_BREAK, this->currentToken));
        } else {
            eat(CONTINUE);
            return make_shared<LoopCtrl>(LoopCtrl(LoopCtrl::LoopCtrlType::CTRL_CONTINUE, this->currentToken));
        }
    }

    shared_ptr<AST> Parser::IDHead() {
        Token token = this->currentToken;
        Token follow = this->lexer->peekNextToken();
        if (follow.getType() == LPAR) {
            return functionCall();
        } else {
            /*
             * assignment or expression
             * it's hard for LL parser to differentiate them
             * so stash Lexer and try to find EQUAL
             */
            Token token_stashed = this->currentToken;
            Token last_token_stashed = this->lastToken;
            Lexer *backup = new Lexer();
            this->lexer->stash(backup);

            variable();
            Token next_token = this->currentToken;

            this->lexer->restore(backup);
            delete backup;
            this->currentToken = token_stashed;
            this->lastToken = last_token_stashed;

            if (next_token.getType() == EQUAL) {
                return assignment();
            } else {
                shared_ptr<AST> ast = expr();
                return make_shared<BlockExpr>(BlockExpr(token, ast));
            }
        }
    }

    Type Parser::eatType() {
        PARSE_LOG(INNERTYPE);

        Type ret;

        if (token_to_simple_types.find(this->currentToken.getType()) != token_to_simple_types.end()) {
            TokenType token = this->currentToken.getType();
            eat(token);
            auto ty = token_to_simple_types[token];
            ret = Type(ty, type_name[ty]);
        } else if (this->currentToken.getType() == ARR) {
            eat(ARR);
            eat(LSQB);
            if (this->currentToken.getType() != RSQB) {
                // Type can be any types, even another vector
                Type nest = eatType();
                if (nest.first == VOID_TY) {
                    nest.first = I8_TY;
                }

                eat(COLON);
                int array_size = 0;
                if (this->currentToken.getType() == INTEGER) {
                    array_size = this->currentToken.getValue().any_cast<int>();
                    eat(INTEGER);
                } else {
                    throw ExceptionFactory<SyntaxException>(
                            "array size must be provided",
                            this->currentToken.line,
                            this->currentToken.column
                    );
                }
                eat(RSQB);
                if (array_size != 0) {
                    llvm::Type *Ty = llvm::ArrayType::get(nest.first, array_size);
                    type_name[Ty] = "arr[" + type_name[nest.first] + ":" + to_string(array_size) + "]";
                    type_name[Ty->getPointerTo()] = type_name[Ty] + "*";
                    type_size[Ty] = type_size[nest.first] * array_size;
                    type_name[Ty->getPointerTo()] = PTR_SIZE;
                    ret = Type(Ty, "arr");
                } else {
                    llvm::Type *Ty = nest.first->getPointerTo();
                    type_name[Ty] = type_name[nest.first] + "*";
                    type_name[Ty->getPointerTo()] = type_name[Ty] + "*";
                    type_size[Ty] = PTR_SIZE;
                    type_name[Ty->getPointerTo()] = PTR_SIZE;
                    ret = Type(Ty, "arr");
                }
            } else {
                throw ExceptionFactory<SyntaxException>(
                        "array type and size must be provided",
                        this->currentToken.line,
                        this->currentToken.column
                );
            }
        } else if (this->currentToken.getType() == VEC) {
            eat(ARR);
            eat(LSQB);
            if (this->currentToken.getType() != RSQB) {
                // Type can be any types, even another vector
                Token token = this->currentToken;
                Type nest = eatType();
                if (nest.first == VOID_TY) {
                    nest.first = I8_TY;
                }

                if (simple_types.find(nest.first) == simple_types.end()) {
                    throw ExceptionFactory<LogicException>(
                            "element of vector type must be simple",
                            token.line,
                            token.column
                    );
                }

                eat(COLON);
                int array_size = 0;
                if (this->currentToken.getType() == INTEGER) {
                    array_size = this->currentToken.getValue().any_cast<int>();
                    eat(INTEGER);
                } else {
                    throw ExceptionFactory<SyntaxException>(
                            "array size must be provided",
                            this->currentToken.line,
                            this->currentToken.column
                    );
                }
                eat(RSQB);
                if (array_size != 0) {
                    llvm::Type *Ty = llvm::VectorType::get(nest.first, array_size, false);
                    type_name[Ty] = "vec[" + type_name[nest.first] + ":" + to_string(array_size) + "]";
                    type_name[Ty->getPointerTo()] = type_name[Ty] + "*";
                    type_size[Ty] = type_size[nest.first] * array_size;
                    type_name[Ty->getPointerTo()] = PTR_SIZE;
                    ret = Type(Ty, "vec");
                } else {
                    throw ExceptionFactory<LogicException>(
                        "array size must bigger than 0",
                        this->currentToken.line,
                        this->currentToken.column
                    );
                }
            } else {
                throw ExceptionFactory<SyntaxException>(
                        "array type and size must be provided",
                        this->currentToken.line,
                        this->currentToken.column
                );
            }
        } else if (this->currentToken.getType() == ID) {
            string id = this->currentToken.getValue().any_cast<string>();
            auto modinfo = this->currentToken.getModInfo();

            auto ty = find_struct(modinfo, id);

            if (ty.first == struct_types.end()) {
                throw ExceptionFactory<MissingException>(
                        "missing type '" + id + "'",
                        this->currentToken.line,
                        this->currentToken.column
                );
            }

            Type Ty = Type(ty.first->second->Ty, ty.second);
            eat(ID);
            ret = Ty;
        } else if (this->currentToken.getType() == DEFAULT) {
            eat(DEFAULT);
            Type Ty = Type(nullptr, "default");
            ret = Ty;
        } else {
            throw ExceptionFactory<SyntaxException>(
                    "type is unrecognized",
                    this->currentToken.line,
                    this->currentToken.column
            );
        }

        while (this->currentToken.getType() == STAR && ret.first) {
            eat(STAR);
            llvm::Type *ty = ret.first->getPointerTo();
            string name = ret.second + "*";
            ret.first = ty;
            ret.second = name;
        }

        return ret;
    }
} // namespace AVSI