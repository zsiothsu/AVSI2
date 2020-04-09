/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-09 20:42:17
 * @Description: file content
 */
#include "Interpreter.h"
using namespace std;
using namespace INTERPRETER;

char cmdBuffer[100];

int main(void)
{
    int cnt = 1;
    while(true)
    {
        cout << ">>";
        cin.getline(cmdBuffer,100);
        string line(cmdBuffer);

        try
        {
            Interpreter action = Interpreter(line);
            cout << action.expr() << endl;
        }
        catch(Exception& e)
        {
            cout << e.what() << "\t at line " << cnt << endl;
            cin.clear();
            //cin.ignore();
        }

        cnt++;
    }
    return 0;
}