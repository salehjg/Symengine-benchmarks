//
// Created by saleh on 11/30/24.
//

#pragma once

#include <iostream>
#include "utils/timers.h"
#include "utils/mem_usage_tracker.h"

class benchmark_base {
protected:
    const std::string name;
    mem_usage_tracker mem_tracker;

public:
    virtual ~benchmark_base() = default;

    explicit benchmark_base(const std::string& name) :
        name(name),
        mem_tracker(1, 100, "./") {
        mem_tracker.startInSeparateThread();
        std::cout << "==============================================" << std::endl;
        std::cout << "*** Benchmark " << name << " created" << std::endl;
    }

    void Run() {
        std::cout << "*** Benchmark " << name << " started" << std::endl;
        Preparation();
        {
            timer_scope ts(name);
            Workload();
        }
        std::cout << "*** Benchmark " << name << " finished" << std::endl;
    }

    virtual void Preparation() = 0;
    virtual void Workload() = 0;
};
