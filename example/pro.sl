#!/bin/Interpreter

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

    return $m
}

function boolean()
{
    a=true
    b=false
    
    if [ $a == false ] then
        echo 111
    elif [ $b == true ] then
        echo 222
    else
        echo 333
    fi
    if [ $a || $b ] then
        echo 444
    fi
    if [ ! $a && $b ] then
        echo 555
    fi

    if [ true ] then
        max(1,2,3)
    fi
}

echo max(32,74,46)
boolean()