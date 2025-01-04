//
// Created by saleh on 11/30/24.
//

#include "bench05.h"
#include "CFileWriterBase.h"
#include "symengine/symbol.h"
#include "symengine/constants.h"
#include "symengine/add.h"
#include "symengine/basic.h"
#include "symengine/mul.h"
#include "symengine/pow.h"


void bench05::Preparation() {
    for (size_t flat = 0; flat < cfg_L; flat++) {
        id_to_sym[get_symbol_id(0, flat)] = SymEngine::symbol(std::to_string(get_symbol_id(0, flat)));
        id_to_sym[get_symbol_id(1, flat)] = SymEngine::symbol(std::to_string(get_symbol_id(1, flat)));
        id_to_sym[get_symbol_id(2, flat)] = SymEngine::symbol(std::to_string(get_symbol_id(2, flat)));
    }
}

/**
 * This benchmark constructs N number of exprs of form:
 *  expr_i = Sum_{j=0}^{L} (a_j + b_j + c_j)^get_random_integer(min=1, max=P)
 *
 *  So our parameters are:
 *  - N: Number of exprs.
 *  - L: Number of terms in each expr.
 *  - P: Power of each term.
 *
 */
void bench05::Workload() {
    std::cout << "Generating " << cfg_N << " expressions of length " << cfg_L << " and power " << cfg_P << std::endl;
    {
        mem_usage_tracker mem_expr_gen(SAMPLING_INTERVAL_MS, 100, "mem_usage_" + name + ".expr_gen.txt", true);
        for (size_t i = 0; i < cfg_N; i++) {
            SymEngine::RCP<const SymEngine::Basic> expr = SymEngine::zero;
            for (size_t j = 0; j < cfg_L; j++) {
                auto base = SymEngine::add(
                    SymEngine::add(
                        id_to_sym[get_symbol_id(0, j)],
                        id_to_sym[get_symbol_id(1, j)]
                    ),
                    id_to_sym[get_symbol_id(2, j)]
                );
                expr = SymEngine::add(expr, SymEngine::pow(base, SymEngine::integer(get_random_integer(1, cfg_P))));
            }
            //exprs.push_back(SymEngine::expand(expr));
            exprs.push_back(expr);
        }
    }

    CFileWriterBase<size_t, SymEngine::RCP<const SymEngine::Basic>> writer("/tmp/", "bench05", true, true);
    auto read_verify = [&](size_t __retid, size_t index, SymEngine::RCP<const SymEngine::Basic> gold) {
        SymEngine::RCP<const SymEngine::Basic> uut;
        auto tuple = writer.Read(__retid, index);
        size_t uut_index = std::get<0>(tuple);
        uut = std::get<1>(tuple);
        if (uut_index != index) {
            std::cout << "Mismatch in serialization at index " << index << " of RetID "<< __retid<<  std::endl;
            std::cout << "Expected index: " << index << std::endl;
            std::cout << "Got index: " << uut_index << std::endl;
            std::flush(std::cout);
            throw std::runtime_error("Serialization mismatch");
        }

        if (not SymEngine::eq(*uut, *gold)) {
            std::cout << "Mismatch in serialization at index " << index << std::endl;
            std::cout << "Expected: " << gold->__str__() << std::endl;
            std::cout << "Got: " << uut->__str__() << std::endl;
            throw std::runtime_error("Serialization mismatch");
        }
    };
    {
        const auto retid = writer.GenerateRetId();
        for (size_t i = 0; i < 2; i++) {
            writer.Append(retid, {i, exprs[i]});
        }
        read_verify(retid, 0, exprs[0]);
        read_verify(retid, 1, exprs[1]);
        for (size_t i = 2; i < 4; i++) {
            writer.Append(retid, {i, exprs[i]});
        }
        read_verify(retid, 3, exprs[3]);
        read_verify(retid, 2, exprs[2]);
    }

    const auto new_retid = writer.GenerateRetId();
    for (size_t i = 0; i < cfg_N; i++) {
        writer.Append(new_retid, {i, exprs[i]});
    }
    for (size_t i = 0; i < cfg_N; i++) {
        read_verify(new_retid, i, exprs[i]);
    }

}
