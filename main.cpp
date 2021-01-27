/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:46:03
 * @Description: entry for interpreter
 */
#include "./inc/Interpreter.h"
#include "./inc/SemanticAnalyzer.h"
#include "./inc/flags.h"

using namespace std;
using namespace AVSI;

DEFINE_bool(scope, false, "print scope information");
DEFINE_bool(callStack, false, "print call stack");

void setFlags(string name) {
    string UsageMessage = name + " file [--scope] [--callStack]";
    google::SetUsageMessage(UsageMessage);
    string VersionString = "0.0.0 (AVSI)";
    google::SetVersionString(VersionString);
}

int main(int argc, char **argv) {
    setFlags(argv[0]);
    google::ParseCommandLineFlags(&argc, &argv, true);

    if (argc == 1) {
        cout << "AVSI: missing target file." << endl;
        return 0;
    }
    if (argc != 2) {
        cout << "AVSI: too more arguements." << endl;
        return 0;
    }

    char *fileName = argv[1];
    ifstream file;
    file.open(fileName, ios::in);
    if (!file.is_open())
        cout << "AVSI: can't open file '" + string(fileName) + "'" << endl;

    Lexer *lexer = new Lexer(&file);
    Parser *parser = new Parser(lexer);
    SemanticAnalyzer *semanticAnalyzer = new SemanticAnalyzer();
    try {
        AST *tree = parser->parse();
        SymbolTable *symbolTable = semanticAnalyzer->SemanticAnalyze(tree);
        Interpreter *interpreter = new Interpreter(tree, symbolTable);
        interpreter->interpret();

        // delete semanticAnalyzer;
        delete interpreter;
    }
    catch (Exception &e) {
        std::cerr << e.what() << "\t at line " << e.line << " column "
                  << e.column + 1 << '\n';
    }
    return 0;
}
