function fun_with_param(a2,b2,c2)
{
    var1 = a2
    var2 = b2 * 10
    var3 = c2 * 100
    a2 = 0
    b2 = 0
    c2 = 0
}

function fun_without_param()
{
    a1 = 4
    a2 = 5
    a3 = 6
    fun_with_param(a1 + 1, a2 + 2, a3 + 3)
}

a = 1
b = 2
c = 3
fun_without_param()