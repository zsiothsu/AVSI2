###
 # @Author: your name
 # @Date: 1970-01-01 08:00:00
 # @LastEditTime: 2020-05-28 00:15:19
 # @Description: file content
### 

function fun_without_param()
{
    var = 1
    function inner(a)
    {
        var = a
    }
}

function fun_with_param(a,b,d)
{
    var = a
}

a = 1
b = a + 1
fun_with_param(a,b,a)