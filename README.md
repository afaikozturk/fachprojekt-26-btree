# Fachprojekt Scientific Computing 26

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



## Anpassungen im Code

### Änderungen an der Page größe

Größe von `static const uint64_t pageSize = 256U;` auf 4096U geändert.

Änderung in Performance: //hier dann Ergebnisse vergleichen ohne änderung und dann mit

### Änderungen an der Reihenfolge von Attributen in Nodes

`NodeBase *children[maxEntries];` und `Key keys[maxEntries];` wurden in `template <class Key> struct BTreeInner : public BTreeInnerBase` vertauscht.

Änderung in Performance: hat nichts gebracht, teilweise sogar schlechter.

### Änderungen mit Speicherallozierung
Es wurden alle new und deletes Entfernt. Dafür wird jetzt am Anfang der gesammte nötige Speicher angesetzt mit NodePool. Die einzelnen Nodes werden dann nacheinander in dem Pool gepackt und beim Löschen bleibt der Speicher erhalten und ganz am ende wird der ganz Pool freigegeben. 
Dazu wurden einige Aufrufe auf die Pool angepasst. 

Es muss die größe des Pool anbgepasst werden/auf das Benchmark passend gemacht werden. Aktuell nur auf das jetzige hardgecoded.

Änderung in Performance: -0.7 sek und -1 sek; Instruktion: -7,6% und etwas mehr; cycles: -17,11% und -26,9%; Stall: -20,8% und -32,2%

### Änderung mit Multithreading
Die Änderungen sind in der Benchmark ausführung

## Virtuel Enviroment für Plot
$ source .venv/bin/activate

## Änderungen mit Branchless Binary Search (anstatt SIMDs)
Mit SIMD-Befehlen hätten mehrere Daten gleichzeitig verarbeitet werden können und mit `AVX2 (__m256i)` 4 64-Bit-Schlüssel gleichzeitig verglichen werden. Da die Liste sortiert ist, zählen wir einfach, wie viele Elemente kleiner als unser Suchschlüssel ist. Sobald wir auf ein Element stoßen, das größer oder gleich ist, wissen wir den Index (`lowerbound`).

Folgende Flag wurde zu CMake Datei hinzugefügt für AVX-Befehle:

(alt) set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG") --> 
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG -march=native -mavx2")

Da wir aber unser `pageSize` auf 8192 Bytes erhöht haben, hat das eine wichtige Konsequenz für unser `lowerBound`-Suche: Bei 8192 Bytes passen jetzt knapp über 510 Schlüssel in einen einzigen Knoten. 

Weil wir jetzt über 512 Elemente haben, braucht selbst SIMD im schlimmsten Fall ungefähr 128 Durchläufe (512/4). Eine klassische binäre Suche braucht immer nur 9 Durchläufe ($\log_2{(512)} = 9$). Das Problem der binäre Suche war nur die `if/else` (also die Branch Misprediction)

Daher die if/else Block bei der lowerBound Funktion wurde durch eine Bedingungzuweisung (Compiler nutzt CMOV (Conditional Move)-Befehle der CPU.) CPU muss dann nichts mehr vorhersagen. 

