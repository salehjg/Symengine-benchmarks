//
// Created by saleh on 3/5/24.
//

#pragma once

#include "symengine/basic.h"
#include "symengine/pow.h"
#include "symengine/add.h"
#include "symengine/mul.h"
#include "symengine/pow.h"
#include "symengine/symbol.h"
#include "symengine/real_mpfr.h"
#include "symengine/functions.h"
#include "symengine/logic.h"
#include "symengine/printers/strprinter.h"
#include "symengine/visitor.h"


namespace SPruner::ExprSe {
    class CClonedExprReconstruction : public SymEngine::BaseVisitor<CClonedExprReconstruction> {
    public:
        using ResolveSig = std::function<
                const SymEngine::RCP<const SymEngine::Basic>(const std::string &name)
        >;

        SymEngine::RCP<const SymEngine::Basic> Apply(const SymEngine::Basic &b, ResolveSig &&resolveLambda) {
            m_oResolveLambda = resolveLambda;
            b.accept(*this);
            return m_pExprRet;
        }

        void bvisit(const SymEngine::Add &x) {
            if (SubExprExists(x, m_pExprRet)) {
                return;
            }
            SymEngine::umap_basic_num dictOrig = x.get_dict();
            SymEngine::umap_basic_num dictReconstr;

            for (const auto &[k, v]: dictOrig) {
                // 2*a + 3*b + 4
                // dict={a:2, b:3}, coef=4
                k->accept(*this);

                dictReconstr[m_pExprRet] = v;
            }
            // Dont use CTOR, it will lead to a weird crash.
            m_pExprRet = SymEngine::Add::from_dict(x.get_coef(), std::move(dictReconstr));
            AddSubExpr(m_pExprRet);
        }

        void bvisit(const SymEngine::Mul &x) {
            if (SubExprExists(x, m_pExprRet)) {
                return;
            }
            SymEngine::map_basic_basic dictOrig = x.get_dict();
            SymEngine::map_basic_basic dictReconstr;
            for (const auto &[k, v]: dictOrig) {
                // 2*a^x*b^y*c
                // dict={a:x, b:y, c:1}, coef=2
                k->accept(*this);
                auto base = m_pExprRet;
                v->accept(*this);
                auto pow = m_pExprRet;
                dictReconstr[base] = pow;
            }
            // Dont use CTOR, it will lead to a weird crash.
            m_pExprRet = SymEngine::Mul::from_dict(x.get_coef(), std::move(dictReconstr));
            AddSubExpr(m_pExprRet);
        }

        void bvisit(const SymEngine::Pow &x) {
            if (SubExprExists(x, m_pExprRet)) {
                return;
            }
            x.get_base()->accept(*this);
            auto base = m_pExprRet;
            x.get_exp()->accept(*this);
            auto exp = m_pExprRet;

            // Dont use CTOR, it will lead to a weird crash.
            m_pExprRet = SymEngine::pow(base, exp);
            AddSubExpr(m_pExprRet);
        }

        void bvisit(const SymEngine::FunctionSymbol &x) {
            if (SubExprExists(x, m_pExprRet)) {
                return;
            }
            SymEngine::vec_basic argsOrig = x.get_args();
            SymEngine::vec_basic argsReconstr;
            for (const auto &arg: argsOrig) {
                arg->accept(*this);
                argsReconstr.push_back(m_pExprRet);
            }
            // Dont use CTOR, it will lead to a weird crash.
            m_pExprRet = SymEngine::function_symbol(x.get_name(), argsReconstr);
            AddSubExpr(m_pExprRet);
        }

        void bvisit(const SymEngine::Symbol &x) {
            if (SubExprExists(x, m_pExprRet)) {
                return;
            }
            auto name = x.get_name();
            m_pExprRet = m_oResolveLambda(name);
            //logger->info("Symbol: {}, gid: {}, resolved: {}", name, gId, m_oPrinter.apply(m_pExprRet));
            AddSubExpr(m_pExprRet);
        }

        void bvisit(const SymEngine::Basic &x) {
            if (SubExprExists(x, m_pExprRet)) {
                return;
            }
            m_pExprRet = x.rcp_from_this();
            AddSubExpr(m_pExprRet);
        }

    protected:
        inline bool SubExprExists(const SymEngine::Basic &b, SymEngine::RCP<const SymEngine::Basic> &out) {
            // if exists return true and set out to m_mReusableSubExprs[b]
            // otherwise return false and set out to SymEngine::zero
            auto it = m_mReusableSubExprs.find(b.rcp_from_this());
            if (it != m_mReusableSubExprs.end()) {
                out = it->second;
                return true;
            }
            out = SymEngine::zero;
            return false;
        }

        inline void AddSubExpr(const SymEngine::RCP<const SymEngine::Basic> &b) {
            m_mReusableSubExprs[b] = b; //overwrite it
        }

        SymEngine::RCP<const SymEngine::Basic> m_pExprRet;
        SymEngine::umap_basic_basic m_mReusableSubExprs;

        ResolveSig m_oResolveLambda;
        SymEngine::StrPrinter m_oPrinter;
    };
}
