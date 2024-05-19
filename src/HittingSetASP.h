#include <sstream>
#include <set>
#include <vector>
#include <iostream>

#include "InconsistencyMeasureASP.h"
#include "Utils.h"
#include "Parser.h"
#include "Constants.h"
#include <clingo.hh>

std::string addUniqueAtomEvaluationPerInterpretation();
std::string ensureKbMemberToBeTrue();

std::string handle_formulas_in_kb_hs(Kb& kb);
void pl_to_ASP_hs(Formula& formula, std::string formula_name, std::string& rules, int num_formulas);

std::string add_conjunction_rules_hs();
std::string add_disjunction_rules_hs();
std::string add_negation_rules_hs();

int compute_optimum_with_inf(std::string& program);

int hitting_set_measure(Kb& kb);