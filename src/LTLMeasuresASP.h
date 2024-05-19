#include <sstream>
#include <set>
#include <vector>
#include <iostream>

#include "Utils.h"
#include "Parser.h"
#include "Constants.h"
#include "InconsistencyMeasureASP.h"
#include <clingo.hh>

// Author: Isabelle Kuhlmann

//std::string add_atom_rules(Kb& kb);
std::string add_atom_rules_formula(Formula f);
std::string add_truth_values_LTL();
std::string handle_formulas_in_kb_LTL(Kb& kb);
std::string handle_single_formula_in_kb_LTL(Formula f);

std::string add_conjunction_rules_LTL();
std::string add_disjunction_rules_LTL();
std::string add_negation_rules_LTL();
std::string add_next_rules();
std::string add_until_rules();
std::string add_globally_rules();
std::string add_finally_rules();
std::string get_base_program_LTL(Kb& kb);

void ltl_to_asp(Formula& formula, std::string formula_name, std::string& rules);

int contension_measure_LTL(Kb& kb, int m);
int drastic_measure_LTL(Kb& kb, int m);