#include <sstream>
#include <set>
#include <vector>
#include <iostream>

#include "InconsistencyMeasureASP.h"
#include "Utils.h"
#include "Parser.h"
#include "Constants.h"
#include <clingo.hh>

std::string handle_formulas_in_kb_d_sum(Kb& kb);

void pl_to_ASP_d_sum(Formula& formula, std::string formula_name, std::string& rules, int num_formulas);

int sum_distance_measure(Kb& kb);