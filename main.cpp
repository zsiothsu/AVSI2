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
        catch(SyntaxException e)
        {
            cout << e.__str() << " at line " << cnt << endl;
        }

        cnt++;
    }
    return 0;
}