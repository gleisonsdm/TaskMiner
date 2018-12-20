This folder contains scripts that can provide an automatic way to compare parallel and sequential runtime results for a spreadcheet, containing much benchmarks.

The idea is try to measure if the data is confiable or not in a easy way, i.e., looking to the boxplot is possible to have an idea about the standart desviation of the runtimes. We used as a function the (sequential number) / (parallel number).
- Small values represents that parallel programs are faster.
- One is the threshold that differ good and bad results, comparing parallel and sequencial executios.
- Large values represents that sequential programs are faster.

Each small boxplot is a benchmark.
