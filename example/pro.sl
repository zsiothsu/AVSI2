#!/usr/bin/Interpreter

func fun_with_param(a2,b2,c2)
{
    var0 = 1 - 2
    var1 = a2 + b2 / var0
    return true
}

func fun_without_param()
{
    a1 = 4
    a2 = 5
    a3 = 6
    a3 = 1 + fun_with_param(a1, a2 ,a3)
    return fun_with_param(a1 + 1, a2 / 2, a3)  * fun_with_param(1,2,3)
}

a = true
b = false
c = a + b
a = fun_without_param()

echo a