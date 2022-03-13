# function at libavsi.a
function println(x)
function print(x)
function put(x)
function input()

function fun(n)
{
    n = n * 2
    for (i = 1;i <= n;i=i+2) do
        for (j = 0;j < (n - i) / 2;j=j+1) do
            put(32)
        done
        for (j = 0;j < i;j = j + 1) do
            put(42)
        done
        put(10)
    done
}

function entry() {
    n = 0
    n = input()
    fun(n)
}