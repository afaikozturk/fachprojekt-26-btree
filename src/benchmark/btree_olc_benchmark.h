#pragma once

#include <perfcpp/event_counter.h>
#include <benchmark/workload_set.h>
#include <tree/btree_olc.h>
#include <thread>
#include <vector>

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
        
        auto totalSize = _workload[phase].size();

        //Alt sequenziell
        /* auto from = 0U;

        if (phase == benchmark::phase::INSERT)
        {
            for (auto i = from; i < totalSize; ++i)
            {
                const auto &request = _workload[phase][i];
                this->_tree->insert(request.key(), request.value());
            }
        }
        else 
        {
            for (auto i = from; i < totalSize; ++i)
            {
                const auto &request = _workload[phase][i];
                std::int64_t value;
                this->_tree->lookup(request.key(), value);
                builtin::DoNotOptimize(value);
            }
        } */

        //Neu Multithreaded
        if (phase == benchmark::phase::INSERT)
        {
            std::vector<std::thread> threads;
            unsigned numThreads = 8;
            auto chunkSize = totalSize / numThreads;

            for (unsigned t = 0; t < numThreads; t++)
            {
                auto start = t * chunkSize;
                auto end = (t == numThreads - 1) ? totalSize : start + chunkSize;
                threads.push_back(std::thread(&BtreeOLCBenchmark::insertRange, this, start, end));
            }

            for (auto &th : threads)
            { 
               th.join();
            }
        }
        else
        {
            std::vector<std::thread> threads;
            unsigned numThreads = 8;
            auto chunkSize = totalSize / numThreads;

            for (unsigned t = 0; t < numThreads; t++)
            {
                auto start = t * chunkSize;
                auto end = (t == numThreads - 1) ? totalSize : start + chunkSize;
                threads.push_back(std::thread(&BtreeOLCBenchmark::lookupRange, this, start, end));
            }

            for (auto &th : threads)
            { 
               th.join();
            }
        }
    }

    //Hilfsfunktionen für das Multithreading für die insert Phase
    void insertRange(std::uint32_t start, std::uint32_t end)
    {
        for (auto i = start; i < end; ++i)
            {
                const auto &request = _workload[phase::INSERT][i];
                this->_tree->insert(request.key(), request.value());
            }
    }

    //Hilfsfunktionen für das Multithreading für die lookup Phase
    void lookupRange(std::uint32_t start, std::uint32_t end)
    {
        for (auto i = start; i < end; ++i)
            {
                const auto &request = _workload[phase::MIXED][i];
                std::int64_t value;
                this->_tree->lookup(request.key(), value);
                builtin::DoNotOptimize(value);
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
            //extra config für das mulitthreading, damit auch alles mit gezählt wird
            auto config = perf::Config{};
            config.include_child_threads(true);
            
            auto event_counter = perf::EventCounter{ counters, config };

            /// Specify hardware events to count
            
            event_counter.add({"seconds", "instructions", "cycles", "CYCLE_ACTIVITY.STALLS_TOTAL"})
            //event_counter.add({"seconds", "cache-references", "cache-misses"});

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