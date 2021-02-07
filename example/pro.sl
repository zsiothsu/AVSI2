#!/bin/Interpreter

echo "Hello World!"

function max(a, b, c)
{
    m=0

    if [ $a -gt $b ] then
        if [ $a -gt $c ] then
            m=$a
        else
            m=$c
        fi
    else
        if [ $b -gt $c ] then
            m=$b
        else
            m=$c
        fi
    fi

    return m
}

function boolean()
{
    a=true
    b=false
    
    if [ a == false ] then
        echo 111
    elif [ b == true ] then
        echo 222
    else
        echo 333
    fi
    if [ a || b ] then
        echo 444
    fi
    if [ ! a && b ] then
        echo 555
    fi

    if [ true ] then
        max(1,2,3)
    fi
}

function printWhile(num)
{
    i = 1
    while i <= num
    do
        echo i
        i = i + 1
    done
}

function printFor(num)
{
    for (i=1;i <= num;i=i+1)
    do
        echo i
    done
}

function self(n)
{
    if [n >= 0]
    then
        echo n
        self(n-1)
    fi
}

echo max(32,74,46)
boolean()
printWhile(5)
printFor(6)
self(3)