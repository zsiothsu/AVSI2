/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 21:16:38
 * @Description: file content
 */
#include "./inc/Interpreter.h"

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
            // Interpreter
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