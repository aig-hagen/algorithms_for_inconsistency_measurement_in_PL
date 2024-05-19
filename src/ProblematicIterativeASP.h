#include <sstream>
#include <set>
#include <vector>
#include <iostream>

#include "InconsistencyMeasureASP.h"
#include "Utils.h"
#include "Parser.h"
#include "Constants.h"
#include <clingo.hh>

// std::string addUniqueAtomEvaluationPerInterpretation();
// std::string ensureKbMemberToBeTrue();

// std::string handle_formulas_in_kb_mv(Kb& kb);
// std::string handle_formulas_in_kb_mv_new(Kb& kb, std::set<std::string> atoms_in_MUS);
// void pl_to_ASP_mv(Formula& formula, std::string formula_name, std::string& rules, int num_formulas);

// std::string add_conjunction_rules_mv_MSS();
// std::string add_disjunction_rules_mv_MSS();
// std::string add_negation_rules_mv_MSS();

// int compute_optimum_with_inf(std::string& program);

// std::string add_general_rules_iterative();
// std::string add_general_rules_iterative_MSS();
// std::string add_cs_rules_iterative();
// std::string add_cs_rules_iterative_MSS();
// std::string add_subset_sat_rules_iterative();
// std::string add_cs_sat_rules_iterative();
// std::string add_cs_superset_rules_iterative();
// std::string add_saturation_iterative();
// std::string add_saturation_iterative_MSS();

std::string add_cs_rules_iterative_MSS_p();
std::string add_cs_rules_iterative_MSS_p_2();

double problematic_measure_iterative_MSS(Kb& kb);
double problematic_measure_iterative_MSS_2(Kb& kb);