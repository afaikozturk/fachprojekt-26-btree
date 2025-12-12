#pragma once

#include <perfcpp/event_counter.h>
#include <benchmark/workload_set.h>
#include <tree/btree_olc.h>

namespace benchmark {

template <typename Tree> class BtreeOLCBenchmark 
{
public:
    BtreeOLCBenchmark(NumericWorkloadSet &&workload, const std::uint16_t iterations)
        : _workload(std::move(workload)), _iterations(iterations)
    {}

    ~BtreeOLCBenchmark() = default;

    void set_up(phase phase)
    {
        if (phase == phase::INSERT)
        {
            this->_tree = new Tree{};
        }
    }

    void tear_down(phase phase)
    {
        if (phase == phase::MIXED)
        {
            delete std::exchange(this->_tree, nullptr);
        }
    }


    void execute_single_run(const benchmark::phase phase)
    {
        auto from = 0U;
        auto to = _workload[phase].size();

        if (phase == benchmark::phase::INSERT)
        {
            for (auto i = from; i < to; ++i)
            {
                const auto &request = _workload[phase][i];
                this->_tree->insert(request.key(), request.value());
            }
        }
        else 
        {
            for (auto i = from; i < to; ++i)
            {
                const auto &request = _workload[phase][i];
                std::int64_t value;
                this->_tree->lookup(request.key(), value);
                builtin::DoNotOptimize(value);
            }
        }
    }

    void validate_tree()
    {
        for (auto i=0; i<_workload[benchmark::phase::INSERT].size(); ++i)
        {
            const auto &request = _workload[benchmark::phase::INSERT][i];
            std::int64_t value;
            this->_tree->lookup(request.key(), value);
            if(value != request.value())
            {
                std::cout << "Error, the tree does not return the correct value. " << request.value() << " expected, but " << value << " returned!\nAborting!" << std::endl;
                exit(1);
            }
        }
    }

    void execute_benchmark() 
    {
        for (auto i = 0U; i < this->_iterations; ++i)
        {
            /// Initialize the counter
            auto counters = perf::CounterDefinition{"../../src/benchmark/perf_list.csv"};
            auto event_counter = perf::EventCounter{ counters };

            /// Specify hardware events to count
            event_counter.add({"seconds", "instructions", "cycles"});

            /// Create the btree
            this->set_up(phase::INSERT);



            /// Here starts the insert phase.
            event_counter.start();
            this->execute_single_run(phase::INSERT);
            event_counter.stop();
            
            validate_tree();

            /// Print the results
            std::cout << "Insert phase[" << i << "]: ";
            auto result = event_counter.result();
            for (const auto [event_name, value] : result)
            {
                std::cout << event_name << ": " << value << " | ";
            }
            std::cout << std::endl;
            


            /// Here starts the lookup phase.
            event_counter.start();
            this->execute_single_run(phase::MIXED);            
            event_counter.stop();

            /// Print the results
            std::cout << "Lookup phase[" << i << "]: ";
            result = event_counter.result();
            for (const auto [event_name, value] : result)
            {
                std::cout << event_name << ": " << value << " | ";
            }
            std::cout << std::endl;



            /// Destroy the btree
            this->tear_down(phase::MIXED);
        }
    }

private:
    Tree *_tree{nullptr};
    const std::uint16_t _iterations;
    NumericWorkloadSet _workload;
};

} // namespace benchmark