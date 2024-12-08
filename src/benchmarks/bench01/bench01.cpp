//
// Created by saleh on 11/30/24.
//

#include "bench01.h"
#include "symengine/symbol.h"
#include "symengine/constants.h"
#include "symengine/add.h"
#include "symengine/mul.h"
#include "symengine/pow.h"
#include "utils/visitor_sym.h"


void bench01::Preparation() {
    for (size_t flat = 0; flat < cfg_L; flat++) {
        id_to_sym[get_symbol_id(0, flat)] = SymEngine::symbol("a_" + std::to_string(flat));
        id_to_sym[get_symbol_id(1, flat)] = SymEngine::symbol("b_" + std::to_string(flat));
        id_to_sym[get_symbol_id(2, flat)] = SymEngine::symbol("c_" + std::to_string(flat));
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
void bench01::Workload() {
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

    std::cout << "Saving the exprs onto the disk." << std::endl;
    {
        mem_usage_tracker mem_expr_save(SAMPLING_INTERVAL_MS, 100, "mem_usage_" + name + ".expr_save.txt", true);

        for (size_t i = 0; i < cfg_N; i++) {
            // create a binary file and save the data as binary
            auto file = std::ofstream("expr_" + std::to_string(i) + ".bin", std::ios::binary);
            auto data = exprs[i]->dumps();
            file.write(data.c_str(), data.size());
            file.close();
            std::cout << "expr_" << i << " size: " << data.size() << " has been saved" << std::endl;
        }
    }

    std::cout << "Checking for duplicates (symbols)" << std::endl;
    {
        mem_usage_tracker mem_check_duplicates(SAMPLING_INTERVAL_MS, 100, "mem_usage_" + name + ".check_duplicates.txt", true);
        visitor_sym visitor;
        auto count_sym = visitor.apply(exprs);
        std::cout << "Number of duplicate symbols found: " << count_sym << std::endl;
    }

    std::cout << "Wiping everything" << std::endl;
    {
        mem_usage_tracker mem_wipe(SAMPLING_INTERVAL_MS, 100, "mem_usage_" + name + ".wipe.txt", true);
        exprs.clear();
        id_to_sym.clear();
    }

    std::cout << "Loading the exprs from the disk." << std::endl;
    {
        mem_usage_tracker mem_expr_load(SAMPLING_INTERVAL_MS, 100, "mem_usage_" + name + ".expr_load.txt", true);
        for (size_t i = 0; i < cfg_N; i++) {
            auto file = std::ifstream("expr_" + std::to_string(i) + ".bin", std::ios::binary);
            if (!file) {throw std::runtime_error("Cannot open file");}
            size_t data_size;
            file.seekg(0, std::ios::end);
            data_size = file.tellg();
            file.seekg(0, std::ios::beg);
            //std::cout << "expr_" << i << " size: " << data_size << " is being loaded" << std::endl;
            std::vector<char> buffer(data_size);
            file.read(buffer.data(), data_size);
            std::string serialized_data(buffer.begin(), buffer.end());
            //std::cout << "expr_" << i << " size: " << serialized_data.size() << " data ready, deserializing" << std::endl;

            exprs.push_back(SymEngine::Basic::loads(serialized_data));
            file.close();
            std::cout << "Loaded expr_" << i << std::endl;
        }

        // sleep for 5 seconds to see the memory usage
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    std::cout << "Checking for duplicates (symbols) again" << std::endl;
    {
        mem_usage_tracker mem_check_duplicates(SAMPLING_INTERVAL_MS, 100, "mem_usage_" + name + ".check_duplicates.txt", true);
        visitor_sym visitor;
        auto count_sym = visitor.apply(exprs);
        std::cout << "Number of duplicate symbols found: " << count_sym << std::endl;
    }

}
