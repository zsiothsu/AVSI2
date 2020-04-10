/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-04-10 20:39:09
 * @Description: some methods for Interpreter class
 */

#include "../inc/Interpreter.h"

namespace INTERPRETER
{
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    /**
     * @description:    default constructor
     * @param:          None
     * @return:         None
     */
    Interpreter::Interpreter(void)
    {
    }

    Interpreter::Interpreter(Parser* paser)
    {
        this->parser = paser;
    }

    /**
     * @description:    default destructor
     * @param:          None
     * @return:         None
     */
    Interpreter::~Interpreter()
    {
    }

    void Interpreter::interpret(int* ans)
    {
        
    }
}