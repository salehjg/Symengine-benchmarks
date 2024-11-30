//
// Created by saleh on 11/30/24.
//

#pragma once

#include "benchmark_base.h"

class bench01: public benchmark_base {
protected:

public:
    bench01() : benchmark_base("bench01") {}

    void Preparation() override;

    void Workload() override;
};


