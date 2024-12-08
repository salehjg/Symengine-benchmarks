//
// Created by saleh on 9/13/23.
//

#pragma once

#include <symengine/basic.h>
#include <symengine/pow.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/symbol.h>
#include <symengine/real_mpfr.h>
#include <symengine/functions.h>
#include <symengine/logic.h>
#include <symengine/printers/strprinter.h>
#include <symengine/visitor.h>


class visitor_sym: public SymEngine::BaseVisitor<visitor_sym> {
public:
    size_t apply(const SymEngine::vec_basic &src_exprs) {
        vec_src = src_exprs;
        count_duplicates = 0;
        for (auto &p : src_exprs) {
            p->accept(*this);
        }
        std::cout << "Total number of symbols registered: " << vec_sym.size() << std::endl;
        return count_duplicates;
    }

    /*
    void bvisit(const SymEngine::Add &x) {

    }

    void bvisit(const SymEngine::Mul &x) {

    }

    void bvisit(const SymEngine::FunctionSymbol &x) {

    }
    */

    void bvisit(const SymEngine::Symbol &x) {
        // Check if the symbol is in the list of symbols
        // If same name exists but the address is different, then we have a duplicate
        bool found = false;
        bool multiple = false;
        for (size_t i = 0; i < vec_sym.size(); i++) {
            auto *p = vec_sym[i].get();
            auto *p_sym = static_cast<const SymEngine::Symbol*>(p);
            if (x.get_name() == p_sym->get_name()) {
                if (found) {
                    multiple = true;
                    break;
                }
                found = true;
            }
        }

        for (size_t i = 0; i < vec_sym.size(); i++) {
            auto *p = vec_sym[i].get();
            auto *p_sym = static_cast<const SymEngine::Symbol*>(p);
            if (x.get_name() == p_sym->get_name()) {
                if (&x != vec_sym[i].get()) {
                    std::cout << "Duplicate symbol found: " << x.get_name() << std::endl;
                    std::cout << "\t|___> Address 1: " << &x << ", Address 2: " << vec_sym[i].get() << std::endl;
                    count_duplicates++;
                }
            }
        }

        if (multiple && found) {
            std::cout << "Multiple symbols found: " << x.get_name() << std::endl;
        }

        if (!found) {
            vec_sym.push_back(x.rcp_from_this());
        }
    }


    void bvisit(const SymEngine::Basic &x) {
        for (auto &p : x.get_args()) {
            p->accept(*this);
        }
    }


protected:
    SymEngine::vec_basic vec_src;
    SymEngine::vec_basic vec_sym;
    size_t count_duplicates;
};
