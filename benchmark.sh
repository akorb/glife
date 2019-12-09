#!/bin/bash

echo 'close all' > benchmark.m
echo 'x = [ 1 2 4 8 16 32 64 128 ];' >> benchmark.m

do_benchmark() {
    echo -n "$1_$2x$3 = [ " >> benchmark.m
    for nprocs in "1" "2" "4" "8" "16" "32" "64" "128";
    do ./glife sample_inputs/$1 $nprocs 10000 $2 $3 >> benchmark.m; done
    echo '];' >> benchmark.m
    echo "plot(x, $1_$2x$3, '$4-o', 'DisplayName', '$1_$2x$3', 'LineWidth', 1.35, 'MarkerFaceColor', '$4')" >> benchmark.m
    echo 'hold on' >> benchmark.m
}

do_benchmark 'make' 1000 1000 'r'
do_benchmark 'make' 100 100 'b'

do_benchmark 'puf' 1000 1000 'k'
do_benchmark 'puf' 300 300 'g'

do_benchmark 'glider' 10000 200 'm'
do_benchmark 'glider' 200 10000 'c'
echo "xlabel('# threads')"
echo "ylabel('time [s]')"
echo 'hold off' >> benchmark.m
echo "legend('Interpreter', 'none')" >> benchmark.m