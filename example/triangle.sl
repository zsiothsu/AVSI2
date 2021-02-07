function fun(n)
{
    n = n * 2
    for (i = 1;i <= n;i=i+2) do
        for (j = 0;j < (n - i) / 2;j=j+1) do
            printf " "
        done
        for (j = 0;j < i;j = j + 1) do
            printf "*"
        done
        echo ""
    done
}

n = 0
printf "Enter the number of triangle layers: "
input n
fun(n)