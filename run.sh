#!/bin/bash

build () {
    if [[ "$*" == *"-clean" ]]; then
        rm -r build -y
    fi

    mkdir -p build
    cd build
    cmake ..
    make
    cd ..
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
    else
        ./b_tree
    fi
    cd ..
}



build_flag=false
workload_flag=false
run_flag=false
clean_flag=false
store_flag=false

for arg in "$@"; do
    case $arg in
        -build) build_flag=true ;;
        -workload) workload_flag=true ;;
        -run) run_flag=true ;;
        -clean) clean_flag=true ;;
        -store) store_flag=true ;;
    esac
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
