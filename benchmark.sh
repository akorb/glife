#!/bin/bash

echo 'close all' > benchmark.m
echo 'x = [ 1 2 4 8 16 32 64 128 ];' >> benchmark.m

do_benchmark() {
    echo -n "$1_$2x$3 = [ " >> benchmark.m
    for nprocs in "1" "2" "4" "8" "16" "32" "64" "128";
    do ./glife sample_inputs/$1 $nprocs 10000 $2 $3 >> benchmark.m; done
    echo '];' >> benchmark.m
    echo "plot(x, $1_$2x$3, 'r-*', 'DisplayName', '$1_$2x$3')" >> benchmark.m
    echo 'hold on' >> benchmark.m
}

do_benchmark 'make' 1000 1000
do_benchmark 'make' 100 100

do_benchmark 'puf' 1000 1000
do_benchmark 'puf' 100 100

do_benchmark 'glider' 10000 200
do_benchmark 'glider' 200 10000

echo 'hold off' >> benchmark.m
echo "legend('Interpreter', 'none')" >> benchmark.m