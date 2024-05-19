#include "SatSolver.h"

#include <sstream>
#include <stdio.h>
#include <chrono>
#include "Utils.h"

bool SatSolver::IsSatisfiable(const Kb &k, bool is_in_cnf)
{
    // measure time it takes to construct the cnf from the encoding
    auto cnf_transform_start = std::chrono::steady_clock::now();
    Formula cnf = Formula(Type::AND);
    if (!is_in_cnf)
    {
      this->mappings_.clear();
      cnf = k.GetTseitinEncoding();
    }
    else
    {
      auto kclauses = k.GetFormulas();
      for (auto j = kclauses.begin(); j != kclauses.end(); ++j)
      {
        cnf.AddSubformula(Formula(*j));
      }
    }
    if (cnf.empty())
    {
      return true;
    }
    auto cnf_transform_end = std::chrono::steady_clock::now();
    double interval = double (std::chrono::duration_cast<std::chrono::microseconds>(cnf_transform_end - cnf_transform_start).count())
                      / double (1000000.0);
    time_msrs::cnf_transform_times.push_back(interval);
    // transforming the encoding to cnf is now done and stored in the variable "cnf"

    //measure how large the encoding is in bytes
    time_msrs::encoding_sizes.push_back(double (sizeof(cnf)));

    auto solver_start_total = std::chrono::steady_clock::now();
    CaDiCaL::Solver *solver = new CaDiCaL::Solver;
    solver->set("quiet", 1);

    std::vector<int> clauses;
    CadicalAddClauses(clauses, cnf);
    bool last_was_zero = false;
    for (int cl : clauses)
    {
        if (last_was_zero && cl == 0)
        {
            // CadicalAddClauses sometimes generates double line endings (zeros)
            // ignore the redundant zeros
            continue;
        }
        else if (cl == 0)
        {
            solver->add(0);
            last_was_zero = true;
        }
        else
        {
            solver->add(cl);
            last_was_zero = false;
        }
    }
    // add final 0
    if (!last_was_zero)
    {
        solver->add(0);
    }
    time_msrs::num_solver_calls++;
    // take timestamp when cadical preprocessing is done and the actual solving starts
    auto solver_start_pure = std::chrono::steady_clock::now();
    int r = solver->solve();
    auto solver_end = std::chrono::steady_clock::now();
    // Compute time interval of the complete solving process (preprocessing + solving)
    double solver_interval_total = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start_total).count())
                      / double (1000000.0);
    time_msrs::solver_times_total.push_back(solver_interval_total);

    // Compute time interval of purely the solving part of the overall solving process
    double solver_interval_pure = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start_pure).count())
            / double (1000000.0);
    time_msrs::solver_times_pure.push_back(solver_interval_pure);

    delete solver;
    if (r == 10)
    {
        return true;
    }
    else if (r == 20)
    {
        return false;
    }
    else
    {
        std::stringstream msg;
        throw std::runtime_error("Solver Error: CaDiCal result is inconclusive");
        return false;
    }
}

void SatSolver::CadicalAddClauses(std::vector<int> &vars, const Formula &f)
{
    if (f.IsLiteral())
    {
        int literal = 1;
        std::string name = f.GetName();
        if (f.IsNegation())
        {
            Formula inner_formula = Formula(*(f.GetSubformulas().begin()));
            name = inner_formula.GetName();
        }
        if (this->mappings_.empty())
        {
            (this->mappings_).emplace(name, literal);
        }
        else if (this->mappings_.find(name) == this->mappings_.end())
        {
            literal = GetMaxEntryInMap<std::string>(this->mappings_) + 1;
            (this->mappings_).emplace(name, literal);
        }
        literal = (this->mappings_).at(name);
        if (f.IsNegation())
        {
            literal = -literal;
        }
        vars.push_back(literal);
    }
    else if (f.IsContradiction())
    {
        vars.push_back(1);
        vars.push_back(0);
        vars.push_back(-1);
        vars.push_back(0);
    }
    else if (f.IsConjunction())
    {
        auto subformulas = f.GetSubformulas();
        for (auto j = subformulas.begin(); j != subformulas.end(); ++j)
        {
            Formula temp = Formula(*j);
            CadicalAddClauses(vars, temp);
            vars.push_back(0);
        }
    }
    else if (f.IsDisjunction())
    {
        auto subformulas = f.GetSubformulas();
        bool all_false = true;
        for (auto j = subformulas.begin(); j != subformulas.end(); ++j)
        {
            Formula temp = Formula(*j);
            if (temp.IsTautology())
            {
                vars.push_back(1);
                vars.push_back(-1);
                continue;
            }
            else if (temp.IsContradiction())
            {
                continue;
            }
            else
            {
                all_false = false;
                CadicalAddClauses(vars, temp);
            }
        }
        if (all_false)
        {
            vars.push_back(1);
            vars.push_back(0);
            vars.push_back(-1);
        }
    }
    else if (!f.IsTautology() && !f.empty())
    {
        std::stringstream msg;
        msg << "Solver Error: Input Formula is not in CNF: " << f;
        throw std::runtime_error(msg.str().c_str());
    }
}
