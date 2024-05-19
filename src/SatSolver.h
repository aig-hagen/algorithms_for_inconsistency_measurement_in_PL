#ifndef SATSOLVER_H
#define SATSOLVER_H

#include "../lib/cadical/src/cadical.hpp" //TODO: Replace relative path
#include "Formula.h"
#include "Kb.h"
#include <unordered_map>
#include <vector>
#include <string>

// This class is a wrapper for the
// CaDiCaL solver.
class SatSolver
{
  private:
    std::unordered_map<std::string, int> mappings_; // this maps atoms to their cadical names (integers)
    void CadicalAddClauses(std::vector<int> &vars, const Formula &f);

  public:
    // If is_in_cnf is false, convert the given knowledge base
    // into CNF using Tseitin's method.
    // Then call the solver
    // to check for satisfiability
    // and return the result.
    bool IsSatisfiable(const Kb &k, bool is_in_cnf);
};

#endif /* SATSOLVER_H */
