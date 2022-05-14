mod com::avsi::mod1

function entry() {
    cond = true
    for(i = 0; i < 10;i = i + 1) do
        if [ i >= 5 && cond] then
            break
        fi
    done
    
    i = 10
    while [ i > 0] do 
        if [ i >= 5] then
            i = i - 1
        else 
            break
        fi
    done
}