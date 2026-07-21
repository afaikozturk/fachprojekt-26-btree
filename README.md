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

(Stand 16.07) Größe von `static const uint64_t pageSize = 256U;` auf idealerweise `1024U` bzw. `2048U` geändert (Sweet Spot für SIMD).

Änderung in Performance: //hier dann Ergebnisse vergleichen ohne änderung und dann mit

### Änderungen an der Reihenfolge von Attributen in Nodes

`NodeBase *children[maxEntries];` und `Key keys[maxEntries];` wurden in `template <class Key> struct BTreeInner : public BTreeInnerBase` vertauscht.

Änderung in Performance: hat nichts gebracht, teilweise sogar schlechter.
(Stand 16.07) In Kombination mit SIMD und Prefetching führt diese Data-Locality (Schlüssel liegen direkt am Anfang der Cache-Line) nun aber zu einer messbaren Verbesserung.

### Änderungen mit Speicherallozierung
Es wurden alle new und deletes Entfernt. Dafür wird jetzt am Anfang der gesammte nötige Speicher angesetzt mit NodePool. Die einzelnen Nodes werden dann nacheinander in dem Pool gepackt und beim Löschen bleibt der Speicher erhalten und ganz am ende wird der ganz Pool freigegeben. 
Dazu wurden einige Aufrufe auf die Pool angepasst. 

Es muss die größe des Pool anbgepasst werden/auf das Benchmark passend gemacht werden. Aktuell nur auf das jetzige hardgecoded. wurde noch einmal angepasst -> sollte insgesammt besser geschrieben werden
Es muss die größe des Pool angepasst werden/auf das Benchmark passend gemacht werden. Aktuell nur auf das jetzige hardgecoded.

Änderung in Performance: -0.7 sek und -1 sek; Instruktion: -7,6% und etwas mehr; cycles: -17,11% und -26,9%; Stall: -20,8% und -32,2%

### Änderung mit Multithreading
Die Änderungen sind in der Benchmark ausführung.

`excecute_single_run` wurde überarbeitet, indem threads erstellt werden und jeder dann nur einen jeweiligen Teil der Arbeit ausführt. Dafür gibt es noch zwei extra Hilfsmethoden je eine pro phase.

Aktuell nur mit build erfolgreich durchlaufen, rest erst bei der nächsten Sitzung testbar.

Änderungen in Performance: sek: 2,26x und 4,73x schneller, Stallanteil um 22% und 14% höcher (nicht gut). Cycles pro instruktion 2,95x und 1,5x (auch nicht gut). Noch mit cache misses und cache referneces getestet, da ist beides um ca 30% runter gegangen. Weitere nachprüfungen über genaue Ursache möglich.

Änderung beim Laptop von Maximilian: sekunden stark runter gegangen 4,2x und 5,9x schneller (perfekt bei 8 threads wäre 8x schneller), aber dafür alles andere etwas höcher. Stall z.B. hat sich proportional zu mehr cycles nicht verändert (nur bei insert mit MT etwas höcher).

## Virtuel Enviroment für Plot
$ source .venv/bin/activate

## Änderungen mit SIMD Linearsuche

Anstatt einer Standard-Binärsuche nutzen wir in der `lowerBound`-Funktion (sowohl für `BTreeLeaf` als auch für `BTreeInner`) nun eine vektorisierte lineare Suche mit SIMD-Befehlen (AVX2). Durch **Loop Unrolling** vergleichen wir 8 64-Bit-Schlüssel gleichzeitig (mittels zweier `__m256i` Vektoren). Mit `_mm256_movemask_pd` und `__builtin_ctz` finden wir den Index ohne teure Branch-Mispredictions. Als Fallback dient weiterhin eine klassische do-while-Binärsuche.

Folgende Flag wurde zu CMake Datei hinzugefügt für AVX-Befehle:

(alt) `set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG")` -->
(neu) `set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG -march=native -mavx2")`

Die Linearsuche verliert bei zu großen Knoten an Effizienz. Bei 8192 Bytes bräuchte SIMD im schlimmsten Fall über 64 Durchläufe. Daher wurde die `pageSize` auf den Sweet Spot von 1024U bzw. 2048U angepasst. So passt der Knoten optimal in den L1-Cache und lässt sich in wenigen Taktzyklen durchsuchen.

## Memory Latency Hiding durch Prefetching

Um Memory Stalls durch Cache-Misses ("Pointer Chasing") zu reduzieren, wurde Software-Prefetching (`__builtin_prefetch(node, 0, 3)`) in die Baumtraversierung (`lookup`, `insert`, `scan`) integriert.
Sobald das nächste Kind ermittelt wurde, wird ein Prefetch abgesetzt. Während die CPU den Optimistic Lock des aktuellen Knotens verifiziert, wird der nächste Knoten bereits asynchron in den schnellen L1-Cache geladen.
