#include "InconsistencyMeasureASP.h"
#include "ForgettingBasedASP.h"
#include "ContensionASP.h"
#include "HittingSetASP.h"
#include "MaxDistanceASP.h"
#include "SumDistanceASP.h"
#include "HitDistanceASP.h"
#include "LTLMeasuresASP.h"
#include "MvIterativeASP.h"
#include "ProblematicIterativeASP.h"
#include "mvpMUSBased.h"
#include <iostream>

using namespace constants;

double get_inconsistency_value(Kb& k, ImSettings config)
{
    if (config.measure_name == "contension")
    {
        return contension_measure(k);
    }
    else if (config.measure_name == "forget")
    {
        return forgetting_based_measure(k);
    }
    else if (config.measure_name == "hs")
    {
        return hitting_set_measure(k);
    }
    else if (config.measure_name == "hitdalal")
    {
        return hit_distance_measure(k);
    }
    else if (config.measure_name == "maxdalal")
    {
        return max_distance_measure(k);
    }
    else if (config.measure_name == "sumdalal")
    {
        return sum_distance_measure(k);
    }
	else if (config.measure_name == "contension-ltl")
    {
        return contension_measure_LTL(k,config.m);
    }
    else if (config.measure_name == "drastic-ltl")
    {
        return drastic_measure_LTL(k,config.m);
    }
	else if (config.measure_name == "mv")
    {
        return mv_measure_iterative(k);
    }
    else if (config.measure_name == "mv2")
    {
        return mv_measure_optimize(k);
    }
    else if (config.measure_name == "mv3b")
    {
        return mv_measure_iterative_2(k);
    }
    else if (config.measure_name == "mv3")
    {
        return mv_measure_iterative_3(k);
    }
    else if (config.measure_name == "mv-mss")
    {
        return mv_measure_iterative_MSS(k);
    }
    else if (config.measure_name == "mv-mss2")
    {
        return mv_measure_iterative_MSS_2(k);
    }
    else if (config.measure_name == "mv-mus")
    {
        return mv_measure_MUS_based(k);
    }
    else if (config.measure_name == "p")
    {
        return problematic_measure_iterative_MSS(k);
    }
    else if (config.measure_name == "p-2")
    {
        return problematic_measure_iterative_MSS_2(k);
    }
    else if (config.measure_name == "p-mus")
    {
        return p_measure_MUS_based(k);
    }
    else //this should not happen
    {
        throw std::runtime_error("Unknown inconsistency measure");
    }
    return -1;
}

std::string add_atom_rules(Kb& kb){
    std::set<std::string> signature = kb.GetSignature();
    std::string atom_rules = "";
    for(std::string atom : signature){
        std::transform(atom.begin(), atom.end(), atom.begin(),
            [](unsigned char c){ return std::tolower(c); });
        std::string atom_rule = ATOM + "(" + atom + ").";
        atom_rules += atom_rule;
    }
    return atom_rules;
}

bool answerSetExists(std::string& program){
    Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
            std::cerr << message << std::endl;
        };
    Clingo::StringSpan string_span{"--opt-mode=opt"};
    Clingo::Control control{string_span, logger, 20};
    const char * program_char_array = program.c_str();

    control.add("base", {}, program_char_array);
    control.ground({{"base", {}}});

    // compute models:
    Clingo::SolveHandle sh = control.solve();
    Clingo::Model m = sh.model();

    if(m){
        // std::cout << m << std::endl;
        return true;
    }
    else{
        return false;
    }

}

std::set<std::string> getNumberOfAtomsInMUS(std::string& program){
    Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
            std::cerr << message << std::endl;
        };
    Clingo::StringSpan string_span{"--opt-mode=opt"};
    Clingo::Control control{string_span, logger, 20};
    const char * program_char_array = program.c_str();

    control.add("base", {}, program_char_array);
    control.ground({{"base", {}}});

    // compute models:
    Clingo::SolveHandle sh = control.solve();
    Clingo::Model m = sh.model();

    std::set<std::string> atom_list;

    if(m){
        for (auto atom : m.symbols()){
            if (atom.match("atomInCs",1)){
                for(auto arg : atom.arguments()){
                    atom_list.insert(arg.name());
                }
            }
        }
        return atom_list;
    }
    else{
        return atom_list;
    }

}

std::set<std::string> getAtomsInMCS(std::string& program){
    Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
            std::cerr << message << std::endl;
        };
    Clingo::StringSpan string_span{"--opt-mode=opt"};
    Clingo::Control control{string_span, logger, 20};
    const char * program_char_array = program.c_str();

    control.add("base", {}, program_char_array);
    control.ground({{"base", {}}});

    // compute models:
    Clingo::SolveHandle sh = control.solve();
    Clingo::Model m = sh.model();

    std::set<std::string> atom_list;

    if(m){
        for (auto atom : m.symbols()){
            if (atom.match("atomInRemainder",1)){
                for(auto arg : atom.arguments()){
                    atom_list.insert(arg.name());
                }
            }
        }
        return atom_list;
    }
    else{
        return atom_list;
    }

}

std::set<std::string> getFormulasInMCS(std::string& program){
    Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
            std::cerr << message << std::endl;
        };
    Clingo::StringSpan string_span{"--opt-mode=opt"};
    Clingo::Control control{string_span, logger, 20};
    const char * program_char_array = program.c_str();

    control.add("base", {}, program_char_array);
    control.ground({{"base", {}}});

    // compute models:
    Clingo::SolveHandle sh = control.solve();
    Clingo::Model m = sh.model();

    std::set<std::string> atom_list;

    if(m){
        for (auto atom : m.symbols()){
            if (atom.match("notInCs",1)){
                for(auto arg : atom.arguments()){
                    atom_list.insert(arg.name());
                }
            }
        }
        return atom_list;
    }
    else{
        return atom_list;
    }

}

std::set<std::string> getFormulasInMUS(std::string& program){
    Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
            std::cerr << message << std::endl;
        };
    Clingo::StringSpan string_span{"--opt-mode=opt"};
    Clingo::Control control{string_span, logger, 20};
    const char * program_char_array = program.c_str();

    control.add("base", {}, program_char_array);
    control.ground({{"base", {}}});

    // compute models:
    Clingo::SolveHandle sh = control.solve();
    Clingo::Model m = sh.model();

    std::set<std::string> atom_list;

    if(m){
        for (auto atom : m.symbols()){
            if (atom.match("inCs",1)){
                for(auto arg : atom.arguments()){
                    atom_list.insert(arg.name());
                    // std::cout << "Found " << arg.name() << std::endl;
                }
            }
        }
        return atom_list;
    }
    else{
        return atom_list;
    }

}

int compute_optimum(std::string& program){

    Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
            std::cerr << message << std::endl;
        };
    Clingo::StringSpan string_span{"--opt-mode=opt"};
    Clingo::Control control{string_span, logger, 20};
    const char * program_char_array = program.c_str();

    control.add("base", {}, program_char_array);
    control.ground({{"base", {}}});

    // compute models:
    Clingo::SolveHandle sh = control.solve();
    Clingo::Model m = sh.model();

    // get minimum value:
    int min_val = m.cost().at(0);
    while(sh.next()){
        Clingo::Model next_m = sh.model();
        if(next_m.cost().at(0) < min_val)
            min_val = m.cost().at(0);
    }

    return min_val;
}

int compute_optimum_with_inf(std::string& program){
    Clingo::Logger logger = [](Clingo::WarningCode, char const *message) {
            std::cerr << message << std::endl;
        };
    Clingo::StringSpan string_span{"--opt-mode=opt"};
    Clingo::Control control{string_span, logger, 20};
    const char * program_char_array = program.c_str();

    control.add("base", {}, program_char_array);
    control.ground({{"base", {}}});

    // compute models:
    Clingo::SolveHandle sh = control.solve();

    Clingo::SolveResult sr = sh.get();
    if(!sr.is_satisfiable()){
        return -1;
    }
    else{
        Clingo::Model m = sh.model();

        // get minimum value:
        int min_val = m.cost().at(0);
        while(sh.next()){
            Clingo::Model next_m = sh.model();
            if(next_m.cost().at(0) < min_val)
                min_val = m.cost().at(0);
        }

        return min_val;
    }
}


std::string add_kb_member_rules_dalal(){
    std::string result = TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,N,I," +  TRUTH_VALUE_T + "):-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,I," + TRUTH_VALUE_T + ")," + KB_MEMBER + "(F,N)," + INTERPRETATION + "(I).";

    result += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,N,I," +  TRUTH_VALUE_F + "):-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,I," + TRUTH_VALUE_F + ")," + KB_MEMBER + "(F,N)," + INTERPRETATION + "(I).";

    // integrity constraint:
    result += ":-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,N,I," + TRUTH_VALUE_F + ")," + KB_MEMBER + "(F,N)," + INTERPRETATION + "(I),N==I.";

    return result;
}

std::string add_dalal_distance_rules(){
    std::string dalal = DIFF + "(A,I1,I2):-" + ATOM + "(A)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,I1,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,I2,Y),X!=Y," + INTERPRETATION + "(I1)," + INTERPRETATION + "(I2).";

    dalal += DALAL + "(I1,I2,X):-X=#count{A:" + DIFF + "(A,I1,I2)," + ATOM + "(A)}," + INTERPRETATION + "(I1)," + INTERPRETATION + "(I2).";

    return dalal;
}

std::string add_conjunction_rules_dalal(){
    std::string conjunction_rules = "";

    // true:
    conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + CONJUNCTION + "(X)," + INTERPRETATION + "(Y),N{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(Z,X)}N," + NUM_CONJUNCTS + "(X,N).";

    // false:
    conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + ").";

    return conjunction_rules;
}

std::string add_disjunction_rules_dalal(){
    std::string disjunction_rules = "";

    // false:
    disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + DISJUNCTION + "(X)," + INTERPRETATION + "(Y),N{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(Z,X)}N," + NUM_DISJUNCTS + "(X,N).";
    // true:
    disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + ").";

    return disjunction_rules;
}

std::string add_negation_rules_dalal(){
    std::string negation_rules = "";

    // true:
    negation_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_F + ").";
    // false:
    negation_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_T + ").";

    return negation_rules;
}
