#!/bin/bash

build () {
    if [[ "$*" == *"-clean" ]]; then
        rm -r build -y
    fi

    mkdir -p build
    cd build
    cmake ..
    make
    make_rc=$?
    cd ..

    if [ $make_rc -ne 0 ]; then
        echo "Compilation failed! Aborting!"
        exit $make_rc
    fi
}

workload () {

    mkdir -p build
    cd build
    cmake ..
    make ycsb
    cp -r workloads/ ../
    rm -r workloads/
    cd ..
}

run () {
    cd build/bin

    if [[ "$*" == *"-store" ]]; then
        ./b_tree > ../../results.csv
        cd ../..
        echo "Results stored in `pwd`/results.csv"
    else
        ./b_tree
        cd ../..
    fi
}



build_flag=false
workload_flag=false
run_flag=false
clean_flag=false
store_flag=false

for arg in "$@"; do
    case $arg in
        --build) build_flag=true ;;
        --workload) workload_flag=true ;;
        --run) run_flag=true ;;
        --clean) clean_flag=true ;;
        --store) store_flag=true ;;
    esac
    if [[ "$arg" == -* && "$arg" != --* && "${#arg}" -gt 1 ]]; then
        for ((i=1; i<${#arg}; i++)); do
            case "${arg:i:1}" in
                b) build_flag=true ;;
                r) run_flag=true ;;
                c) clean_flag=true ;;
                s) store_flag=true ;;
                *) echo "Unknown short flag: -${arg:i:1}"; exit 1 ;;
            esac
        done
    fi
done

if $build_flag; then
    if $clean_flag; then
        build -clean
    else
        build
    fi
fi


if $workload_flag; then
    workload
fi

if $run_flag; then
    if $store_flag; then
        run -store
    else
        run
    fi
fi
