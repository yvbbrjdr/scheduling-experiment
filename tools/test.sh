#!/bin/bash

if [ $# -eq 0 ]
  then
    echo "Need path to se"
    exit
fi

#variables
END_THREAD_NUM=50
THREAD_INCREMENTS=10
END_RATE=100000000
RATE_INCREMENTS=10000
TEST_TIME=10

#plan is to iterate through each of the modes and vary each quantity
#execute for 10 seconds each
PIPE_TYPES=(b s e u)
PIN_MODES=(s r n)
THREAD_NUM=0
RATE=0

mkdir -p results

for type in "${PIPE_TYPES[@]}"
do
    for mode in "${PIN_MODES[@]}"
    do
        for ((n=1;n<=END_THREAD_NUM;n+=THREAD_INCREMENTS))
        do
            for ((r=1;r<=END_RATE;r+=RATE_INCREMENTS))
            do
                echo "Executing $1 $type $mode $n $r"
                cmd="$1 $type $mode $n $r"
                $cmd > "results/$type_$mode_$n_$r" &
                TASK_PID=$!
                sleep $TEST_TIME
                kill $TASK_PID
            done
        done
    done
done