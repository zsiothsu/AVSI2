/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-22 23:06:33
 * @Description: entry for interpreter
 */
#include "./inc/Interpreter.h"
#include "./inc/SemanticAnalyzer.h"
using namespace std;
using namespace AVSI;

char cmdBuffer[100];

int main(int argc,char** argv)
{

    if(argc == 1)
    {
        cout << "AVSI: No input file" << endl;
    }
    else if(argc == 2)
    {
        char* fileName = argv[1];
        ifstream  file;
        file.open(fileName,ios::in);
        if(!file.is_open()) cout<< "AVSI: can't open file '" + string(fileName) + "'"<< endl;

        Lexer* lexer = new Lexer(&file);
        Parser* parser = new Parser(lexer);
        SemanticAnalyzer* semanticAnalyzer = new SemanticAnalyzer();
        Interpreter* interpreter = new Interpreter();
        try
        {
            AST* tree = parser->parse();
            semanticAnalyzer->SemanticAnalyze(tree);
            //interpreter->interpret(tree);
        }
        catch(Exception& e)
        {
            std::cerr << e.what() << "\t at line " << e.line << " column " << e.column + 1 <<'\n';
        }

        delete lexer;
        delete parser;
        delete interpreter;
    }
    else cout << "AVSI: too more arguments" << endl;
    return 0;
}
