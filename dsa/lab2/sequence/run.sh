#!/usr/bin/bash

make benchmark

> output.txt

for ((i=1000; i<=10000; i*=10))
do
    python3 gen.py --type append -n $i
    python3 gen.py --type pop_back -n $i
    python3 gen.py --type random_insert -n $i
    python3 gen.py --type random_remove -n $i
    echo "N=$i..." | tee -a output.txt
    ./benchmark --all >> output.txt
done