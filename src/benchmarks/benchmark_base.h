//
// Created by saleh on 11/30/24.
//

#pragma once

#include <iostream>
#include "utils/timers.h"
#include "utils/mem_usage_tracker.h"

#define SAMPLING_INTERVAL_MS 100

class benchmark_base {
protected:
    const std::string name;
    mem_usage_tracker mem_tracker;


public:
    virtual ~benchmark_base() = default;

    explicit benchmark_base(const std::string& name) :
        name(name),
        mem_tracker(SAMPLING_INTERVAL_MS, 100, "mem_usage_" + name + ".global.txt") {
        std::cout << "==============================================" << std::endl;
        std::cout << "*** Benchmark " << name << " created" << std::endl;
    }

    void Run() {
        std::cout << "*** Benchmark " << name << " started" << std::endl;
        Preparation();
        std::cout << "*** Benchmark " << name << " preparation finished" << std::endl;
        {
            timer_scope ts("Time (ms) spent in Workload for " + name);
            Workload();
        }
        std::cout << "*** Benchmark " << name << " finished" << std::endl;
        //mem_tracker.requestStopAndWait();
    }

    virtual void Preparation() = 0;
    virtual void Workload() = 0;
};
