###
 # @Author: your name
 # @Date: 1970-01-01 08:00:00
 # @LastEditTime: 2020-05-26 16:05:58
 # @Description: file content
### 

function fun_with_param(a,b,d)
{
    var = a
}

function fun_without_param()
{
    var = 1
    function inner(a)
    {
        var = a
    }
}

a = 1
b = a + 1
fun_with_param(a,b,a)