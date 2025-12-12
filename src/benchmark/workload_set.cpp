#include "workload_set.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>

using namespace benchmark;

void NumericWorkloadSet::build(const std::string &insert_workload_file, const std::string &mixed_workload_file)
{
    auto parse = [](auto &file_stream, std::vector<NumericTuple> &data_set) {
        std::srand(1337);
        std::string op_name;
        std::uint64_t key{};

        while (file_stream >> op_name >> key)
        {
            if (op_name == "INSERT")
            {
                data_set.emplace_back(NumericTuple{NumericTuple::Type::INSERT, key, std::rand()});
            }
            else if (op_name == "READ")
            {
                data_set.emplace_back(NumericTuple{NumericTuple::Type::LOOKUP, key});
            }
            else if (op_name == "UPDATE")
            {
                data_set.emplace_back(NumericTuple{NumericTuple::Type::UPDATE, key, std::rand()});
            }
        }
    };

    std::mutex out_mutex;
    auto fill_thread = std::thread{[this, &out_mutex, &parse, &insert_workload_file]() {
        auto workload_file = std::ifstream{insert_workload_file};
        if (workload_file.good())
        {
            parse(workload_file, this->_data_sets[static_cast<std::size_t>(phase::INSERT)]);
        }
        else
        {
            std::lock_guard lock{out_mutex};
            std::cerr << "Could not open workload file '" << insert_workload_file << "'." << std::endl;
        }
    }};

    auto mixed_thread = std::thread{[this, &out_mutex, &parse, &mixed_workload_file]() {
        auto workload_file = std::ifstream{mixed_workload_file};
        if (workload_file.good())
        {
            parse(workload_file, this->_data_sets[static_cast<std::size_t>(phase::MIXED)]);
        }
        else
        {
            std::lock_guard lock{out_mutex};
            std::cerr << "Could not open workload file '" << mixed_workload_file << "'." << std::endl;
        }
    }};

    fill_thread.join();
    mixed_thread.join();
}

namespace benchmark {
std::ostream &operator<<(std::ostream &stream, const NumericWorkloadSet &workload)
{

    stream << "insert phase: " << workload[phase::INSERT].size() << " inserts" << " / "
           << "mixed phase: " << workload[phase::MIXED].size() << " requests";

    return stream << std::flush;
}
} // namespace benchmark
