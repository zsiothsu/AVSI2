func fun_with_param(a2,b2,c2)
{
    var0 = 1 - 1
    var1 = a2 + b2 / var0
    return var1
}

func fun_without_param()
{
    a1 = 4
    a2 = 5
    a3 = 6
    a3 = 1 + fun_with_param(a1 + 1, a2 + 2, a3 + 3)
    return fun_with_param(a1 + 1, a2 / 2, a3)  * fun_with_param(1,2,3)
}

a = 1
b = 2
c = 3
a = fun_without_param()