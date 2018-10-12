#########################################################################
# File Name: bin/das-stop.sh
# Author: xuwenlong
# mail: myxuan475@126.com
# Created Time: 2018年05月04日 星期五 15时14分16秒
#########################################################################
#!/bin/sh

check_script()
{
    echo "check_script $1"
    proc=$(ps l|grep $1|grep -v grep|awk '{print $3}')

    for p in $proc
    do
       echo "check $p"
       expr $p "+" 10 &> /dev/null
    done
    return $?
}

kill_script()
{
    echo "kill_script $1"
    proc=$(ps l|grep $1|grep -v grep|awk '{print $3}')
    for p in $proc
    do
       echo "kill $p"
       kill -s 9 $p
    done
}    

check_process()
{
    echo "check_process $1"
    proc=$(pgrep $1)
    echo "check $proc"
    expr $proc "+" 10 &> /dev/null

    return $?
}

kill_process()
{
    echo "kill_process $2"
    proc=$(pgrep $2)
    for p in $proc
    do
        kill -s $1 $proc
    done
}
    
kill_script das-run.sh

kill_process 2 /leo-das/bin/leo-das
kill_process 9 /leo-das/bin/script-server

ps l|grep das|grep -v grep

echo "------------all killed---------------"
