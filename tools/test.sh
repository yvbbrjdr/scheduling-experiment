#!/bin/bash
set -euo pipefail

if [[ $# -eq 0 ]]; then
    echo "Need path to se"
    exit 1
fi

PIPE_TYPES=(b s e u)
PIN_MODES=(s r n)
PIPE_LEN=(2 4 8 16 100)
RATES=(2000000 20000000)

mkdir -p results

for type in "${PIPE_TYPES[@]}"; do
    for mode in "${PIN_MODES[@]}"; do
        for n in "${PIPE_LEN[@]}"; do
            for r in "${RATES[@]}"; do
                cmd="$1 $type $mode $n $r"
                echo "Executing $cmd"
                FILENAME="results/${type}_${mode}_${n}_${r}"
                timeout --preserve-status -s INT 2 $cmd > "$FILENAME"
                gzip "$FILENAME"
            done
        done
    done
done

lscpu > results/cpu
free -h > results/ram

tar czvf results.tar.gz results
rm -rf results
