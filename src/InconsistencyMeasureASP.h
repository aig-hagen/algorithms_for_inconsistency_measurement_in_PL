#include <clingo.hh>
#include "Utils.h"
#include "Constants.h"

double get_inconsistency_value(Kb& k, ImSettings config);

std::string add_atom_rules(Kb& kb);

int compute_optimum(std::string& program);
int compute_optimum_with_inf(std::string& program);

//BEGIN: functions used in MvIterativeASP.cpp
bool answerSetExists(std::string& program);
std::set<std::string> getNumberOfAtomsInMUS(std::string& program);
std::set<std::string> getAtomsInMCS(std::string& program);
std::set<std::string> getFormulasInMCS(std::string& program);
//END: functions used in MvIterativeASP.cpp

std::string add_kb_member_rules_dalal();
std::string add_dalal_distance_rules();

std::string add_conjunction_rules_dalal();
std::string add_disjunction_rules_dalal();
std::string add_negation_rules_dalal();