/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-18 17:30:14
 * @Description: entry for interpreter
 */
#include "./inc/Interpreter.h"
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
        Interpreter* interpreter = new Interpreter(parser);
        try
        {
            interpreter->interpret();
        }
        catch(Exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        delete lexer;
        delete parser;
        delete interpreter;
    }
    else cout << "AVSI: too more arguments" << endl;
    return 0;
}
