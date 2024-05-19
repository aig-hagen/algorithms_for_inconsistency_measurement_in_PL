#include <sstream>
#include <set>
#include <vector>
#include <iostream>
#include <map>

#include "InconsistencyMeasureASP.h"
#include "Utils.h"
#include "Parser.h"
#include "Constants.h"
#include <clingo.hh>

std::string getAtomOccRules();

std::string handle_formulas_in_kb_fb(Kb& kb, std::map<std::string, int>& atom_occurences);
void pl_to_ASP_fb(Formula& formula, std::string formula_name, std::string& rules, std::map<std::string, int>& atom_occurences);

void initialize_atom_occurences(std::map<std::string, int>& atom_occurences, Kb& kb);

std::string add_conjunction_rules_fb();
std::string add_disjunction_rules_fb();
std::string add_negation_rules_fb();

int forgetting_based_measure(Kb& kb);
