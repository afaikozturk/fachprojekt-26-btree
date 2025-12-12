#include <argparse/argparse.hpp>
#include <benchmark/btree_olc_benchmark.h>
#include <benchmark/workload_set.h>
#include <cstdint>
#include <iostream>

int main(int count_arguments, char **arguments)
{
    auto argument_parser = argparse::ArgumentParser{"b_tree"};

    argument_parser.add_argument("-i", "--iterations")
        .help("Number of iterations for each workload.")
        .default_value(std::uint16_t(1U))
        .action([](const std::string &value) { return std::uint16_t(std::stoi(value)); });
    argument_parser.add_argument("--insert-workload")
        .help("File containing the workload to insert in the tree.")
        .default_value(std::string{"../../workloads/fill_randint_workloada"});
    argument_parser.add_argument("--mixed-workload")
        .help("File containing the workload for the tree lookup.")
        .default_value(std::string{"../../workloads/mixed_randint_workloada"});

    // Parse arguments.
    try
    {
        argument_parser.parse_args(count_arguments, arguments);
    }
    catch (std::runtime_error &e)
    {
        std::cout << argument_parser << std::endl;
        return 1;
    }

    /// Benchmark parameters.
    const auto iterations = argument_parser.get<std::uint16_t>("-i");
    /// Benchmark
    const auto insert_file = argument_parser.get<std::string>("--insert-workload");
    const auto mixed_file = argument_parser.get<std::string>("--mixed-workload");
    auto benchmark_set = benchmark::NumericWorkloadSet{insert_file, mixed_file};

    std::cout << "Workload: " << benchmark_set << std::endl;

    /// Normal BTree
    auto benchmark = benchmark::BtreeOLCBenchmark<btreeolc::BTree<std::uint64_t, std::int64_t>>{std::move(benchmark_set), iterations};
    benchmark.benchmark::BtreeOLCBenchmark<btreeolc::BTree<std::uint64_t, std::int64_t>>::execute_benchmark();

    return 0;
}
