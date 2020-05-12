/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-12 14:58:23
 * @Description: file content
 */
#include "./inc/Interpreter.h"
#include <fstream>
using namespace std;
using namespace AVSI;

char cmdBuffer[100];

AST* tmp(BinOp& b)
{
    AST* c = &b;
    return c;
}

int main(int argc,char** argv)
{

    if(argc == 1)
    {
        int cnt = 1;
        while(true)
        {
            cout << ">>";
            cin.getline(cmdBuffer,100);
            std::string line(cmdBuffer);

            try
            {
                Lexer* lexer = new Lexer(line);
                Parser* parser = new Parser(lexer);
                Interpreter* interpreter = new Interpreter(parser);
                any ans = 0;
                ans = interpreter->interpret();
                cout << ans << endl;

                delete lexer;
                delete parser;
                delete interpreter;
            }
            catch(Exception& e)
            {
                cout << e.what() << "\t at line " << cnt << endl;
                cin.clear();
            }

            cnt++;
        }
    }
    else if(argc == 2)
    {
        char* fileName = argv[1];
        ifstream  file;
        file.open(fileName,ios::in);
        if(!file.is_open()) cout<< "AVSI: can't open file '" + string(fileName) + "'"<< endl;
    }
    else cout << "AVSI: too more arguments" << endl;
    return 0;
}