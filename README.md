# Fachprojekt Scientific Computing 24/25

This repository contains the micro benchmarks for the Fachprojekt Scientific Computing. The goal is to find one or multiple bottlenecks in performance for each of the benchmarks and try to rewrite the code in a way which increases performance.

## How to build and run a benchmark

The `run.sh` script is used to both build and run.
To build the benchmark the flag `-build` is used. An additional flag `-clean` can be used to first clean the build folder and rebuild everything. This can be a useful first step in debugging.

The workload to be used during benchmarking needs to be generated before the first benchmark. This can be done using the flag `-workload`. The workload specifications can be edited in the `workload_specification/workloada`.

To run a benchmark the flag `-run` is used. Per default the results printed to the terminal. If the results should instead be stored in a `result.csv` file, the flag `-store` can be used. All flags can be combined. E.g. first clean building, generating the workload and then running the benchmark without storing to the file, the call is
```console
./run.sh -build -clean -workload -run 
```
The order of the flags is irrelevant, but the benchmark folder has to be the first argument to give.


## Experimental Environment
### Numa Balancing: OFF

    echo 0 > /proc/sys/kernel/numa_balancing

### Performance Governor

    cpupower frequency-set -g performance

### Activate Performance Counter

    sysctl -w kernel.perf_event_paranoid=-1
