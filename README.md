This is an implementation of the classical _Conway's Game of Life_. It has been implemented in C in a parallel fashion. The report.pdf contains an overview of the parallelization approach.

The visualizing in the report has been done with MatLab. I wrote a script called benchmark.sh which creates a MatLab script that plots the results of the benchmark. It requires the source code to be compiled in benchmark mode because there the output of the program is simplified for easier usage in my script.

Compile instructions:

`make`

Benchmark instructions:

```bash
make benchmark
./benchmark.sh
```
