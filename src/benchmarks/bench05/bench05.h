//
// Created by saleh on 11/30/24.
//

#pragma once

#include <unordered_map>

#include "benchmark_base.h"
#include "symengine/basic.h"

class bench05: public benchmark_base {
protected:
    std::unordered_map<size_t, SymEngine::RCP<const SymEngine::Basic>> id_to_sym;
    const size_t cfg_N, cfg_L, cfg_P;
    SymEngine::vec_basic exprs;
public:
    bench05(size_t cfg_N, size_t cfg_L, size_t cfg_P) :
        benchmark_base("bench05"),
        cfg_N(cfg_N), cfg_L(cfg_L), cfg_P(cfg_P)
    {}

    void Preparation() override;

    void Workload() override;

private:
    /**
     * Get the unique symbol id for the symbol across all tensors.
     * @param tid The tensor Id. All tensors are assumed to be of the same size.
     * @param flat_idx The flat index of the tensor element.
     * @return The unique symbol id for the symbol across all tensors.
     */
    size_t get_symbol_id(size_t tid, size_t flat_idx) const {
        return tid * cfg_L + flat_idx;
    }

    size_t get_random_integer(size_t min, size_t max) const {
        return min + (rand() % static_cast<int>(max - min + 1));
    }
};


