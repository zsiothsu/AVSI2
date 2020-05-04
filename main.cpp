/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-04 14:16:15
 * @Description: file content
 */
#include "./inc/Interpreter.h"

using namespace std;
using namespace AVSI;

char cmdBuffer[100];

AST* tmp(BinOp& b)
{
    AST* c = &b;
    return c;
}

int main(void)
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
            double ans = 0;
            interpreter->interpret(&ans);
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
    return 0;
}