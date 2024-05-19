#include <sstream>
#include <set>
#include <vector>
#include <iostream>

#include "InconsistencyMeasureASP.h"
#include "Utils.h"
#include "Parser.h"
#include "Constants.h"
#include <clingo.hh>

std::string handle_formulas_in_kb_d_hit(Kb& kb);

void pl_to_ASP_d_hit(Formula& formula, std::string formula_name, std::string& rules, int num_formulas);

std::string add_conjunction_rules_hit_dalal();
std::string add_disjunction_rules_hit_dalal();
std::string add_negation_rules_hit_dalal();

int hit_distance_measure(Kb& kb);