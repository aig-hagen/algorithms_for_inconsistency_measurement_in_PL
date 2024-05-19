#include <sstream>
#include <set>
#include <vector>
#include <iostream>

#include "Utils.h"
#include "Parser.h"
#include "Constants.h"
#include "InconsistencyMeasureASP.h"
#include <clingo.hh>

std::string add_truth_values();
std::string add_atom_rules(Kb& kb);
std::string handle_formulas_in_kb(Kb& kb);
std::string add_conjunction_rules();
std::string add_disjunction_rules();
std::string add_negation_rules();
std::string add_atomic_formula_rules();

void pl_to_ASP(Formula& formula, std::string formula_name, std::string& rules);

int contension_measure(Kb& kb);