#include "Exception.h"

namespace INTERPRETER
{
    SyntaxException::SyntaxException()
    {
    }
    
    SyntaxException::SyntaxException(string str)
    {
        this->str = str;
    }

    SyntaxException::SyntaxException(string str,int index)
    {
        this->str = str;
        this->line = index;
    }

    SyntaxException::~SyntaxException()
    {
    }

    string SyntaxException::__str()
    {
        return this->str;
    }

    int SyntaxException::__line()
    {
        return this->line;
    }
}