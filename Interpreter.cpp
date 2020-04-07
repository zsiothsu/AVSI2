#include "Interpreter.h"

namespace INTERPRETER
{
    Interpreter::Interpreter()
    {
    }

    Interpreter::~Interpreter()
    {
    }

    Interpreter::Interpreter(string line)
    {
        this->line = line;
        this->cur = 0;
    }

    void Interpreter::eat(Type type)
    {
        if(type == currentToken.getType())
        {
            this->currentToken = getNextToken();
        }
        else
        {
            throw SyntaxException("invalid syntax");
        }
        
    }

    int Interpreter::calc(int left,int right,char opt)
    {
        switch (opt)
        {
        case '+':
            return left + right;
            break;
        case '-':
            return left - right;
            break;
        case '*':
            return left * right;
            break;
        case '/':
            return left / right;
            break;
        default:
            break;
        }
        throw SyntaxException("invalid syntax");
    }

    int Interpreter::expr()
    {
        try{
            this->currentToken = getNextToken();

            int left = currentToken.getValue();
            eat(INT);

            opt opt = currentToken.getValue();
            eat(OPT);

            int right = currentToken.getValue();
            eat(INT);

            return calc(left,right,opt);
        }
        catch(SyntaxException e)
        {
            throw e;
        }
    }

    Token Interpreter::getNextToken()
    {
        try
        {
            char currentChar = this->line[cur];

            while(currentChar == ' ')
            {
                currentChar = this->line[++cur];
            }

            if(isdigit(currentChar))
            {
                int num = 0;

                while(isdigit(currentChar))
                {
                    num = num * 10 + currentChar - '0';
                    if(this->cur < this->line.length())
                        currentChar = this->line[++cur];
                    else break;
                }

                return Token(INT,num);
            }
            else
            {
                Token res;
                switch (currentChar)
                {
                case '+':
                    res = Token(OPT,'+');
                    break;
                case '-':
                    res = Token(OPT,'-');
                    break;
                case '*':
                    res = Token(OPT,'*');
                    break;
                case '/':
                    res = Token(OPT,'/');
                    break;
                default:
                    break;
                }
                this->cur++;
                return res;
            }

            throw SyntaxException("invalid syntax");
        }
        catch(out_of_range e)
        {
            return Token(END,-1);
        }
    }
}