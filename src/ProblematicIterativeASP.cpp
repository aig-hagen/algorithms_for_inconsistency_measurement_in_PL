#include "MvIterativeASP.h"
#include "ProblematicIterativeASP.h"
#include "InconsistencyMeasureASP.h"
#include <chrono>
using namespace constants;


std::string add_cs_rules_iterative_MSS_p(){
    std::string program = "";

    // create candidate set:
    program += "1{" + IN_CS + "(X):" + KB_MEMBER + "(X)}."; // "N:-" + NUM_KB_ELEMENTS + "(N).";

    // Auxiliary predicate that tells whether a formula is NOT in the CS:
    // not_inCs(F) :- kbMember(F), not inCs(F).
    program += NOT_IN_CS + "(F):-" + KB_MEMBER + "(F),not " + IN_CS + "(F).";

    // At least one query formula must be included in at least one formula outside the CS:
    program += "validCS:-1{" + NOT_IN_CS + "(F):" + QUERY_FORMULA + "(F)}.";
    program += ":-not validCS.";

    // atomInCs(A) :- atomInFormula(A,F), inCs(F).
    program += ATOM_IN_CS + "(A):-" + ATOM_IN_FORMULA_MV + "(A,F)," + IN_CS + "(F).";

    // count number of elements in cs:
    program += NUM_ELEMENTS_IN_CS + "(X):-X=#count{F:" + IN_CS + "(F)}.";
    

    return program;
}

std::string add_cs_rules_iterative_MSS_p_2(){
    std::string program = "";

    // create candidate set:
    program += "1{" + IN_CS + "(X):" + KB_MEMBER + "(X)}."; // "N:-" + NUM_KB_ELEMENTS + "(N).";

    // Auxiliary predicate that tells whether a formula is NOT in the CS:
    // not_inCs(F) :- kbMember(F), not inCs(F).
    program += NOT_IN_CS + "(F):-" + KB_MEMBER + "(F),not " + IN_CS + "(F).";

    // // At least one query formula must be included in at least one formula outside the CS:
    // program += "validCS:-1{" + NOT_IN_CS + "(F):" + QUERY_FORMULA + "(F)}.";
    // program += ":-not validCS.";

    // The query formula must be included in the MCS, i.e., it must not be included in the CS:
    program += ":-not " + NOT_IN_CS + "(F)," + QUERY_FORMULA + "(F).";

    // atomInCs(A) :- atomInFormula(A,F), inCs(F).
    program += ATOM_IN_CS + "(A):-" + ATOM_IN_FORMULA_MV + "(A,F)," + IN_CS + "(F).";

    // count number of elements in cs:
    program += NUM_ELEMENTS_IN_CS + "(X):-X=#count{F:" + IN_CS + "(F)}.";
    

    return program;
}


// p
double problematic_measure_iterative_MSS(Kb& kb){

    // This version checks whether a formula is included in an MCS (by checking whether it is contained in a formula in the complement of an MSS)
    // Additionally, the procedure uses potentially fewer iterations by checking which (other) formulas are in the MCSes
    // This version also uses a more "flexible" iteration method by specifying that we are looking for an MCS containing *any* unseen formula.
    // Thus, the iteration stops after no solution is found anymore.

    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        // std::cout << "Inconsistency value: 0" << std::endl; 
        return 0;
    }

    else{

        // iterate over signature
        // for each atom, check if there is an answer set
        std::set<std::string> signature = kb.GetSignature();

        // initialize program string:
        std::string program = "";

        // ---- set up program without query atom ---

        // add number of KB elements:
        program += NUM_KB_ELEMENTS + "(" + std::to_string(numFormulas) + ").";

        // add rules for each formula:
        program += handle_formulas_in_kb_mv(kb);

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_mv_MSS();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_mv_MSS();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_mv_MSS();

        program += add_general_rules_iterative_MSS();
        program += add_cs_rules_iterative_MSS_p();
        program += add_cs_sat_rules_iterative();
        program += add_cs_superset_rules_iterative();
        // program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative_MSS();

        std::set<std::string> formulas_in_MCS; // = kb.GetSignature();

        // std::set<std::string> remaining_atoms_tmp = kb.GetSignature();
        std::set<std::string> remaining_formulas;

        for (int i = 0; i < kb.size(); i++){
            std::string formula_name = FORMULA_PREFIX + std::to_string(i);
            remaining_formulas.insert(formula_name);
        }

        while(!remaining_formulas.empty()){

            // add query formulas:
            std::string all_query_formulas = "";

            for(auto f: remaining_formulas){
                std::string query_formula = QUERY_FORMULA + "(" + f + ").";
                all_query_formulas += query_formula;
            }

            std::string curr_program = program + all_query_formulas;

            // ------------------------------------------------------------------

            auto encoding_end = std::chrono::steady_clock::now();
            double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                              / double (1000000.0);
            time_msrs::encoding_times.push_back(encoding_interval);

            time_msrs::num_solver_calls++;
            // Measure the time clingo takes to solve the program
            auto solver_start = std::chrono::steady_clock::now();
            
            // let Clingo solve the problem; check if an answer set exists:
            // also check which formulas are included in the MIS found
            std::set<std::string> formulas_in_curr_MCS = getFormulasInMCS(curr_program);

            if (formulas_in_curr_MCS.empty()){
                break;
            }

            for(auto a: formulas_in_curr_MCS){
                // std::cout << a << std::endl;
                formulas_in_MCS.insert(a);
                remaining_formulas.erase(a);
            }

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

        }

        return (double)formulas_in_MCS.size();
    }  
}

// p-2
double problematic_measure_iterative_MSS_2(Kb& kb){

    // This version checks whether a formula is included in an MCS (by checking whether it is contained in a formula in the complement of an MSS)
    // Additionally, the procedure uses potentially fewer iterations by checking which (other) formulas are in the MCSes

    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        // std::cout << "Inconsistency value: 0" << std::endl; 
        return 0;
    }

    else{

        // iterate over signature
        // for each atom, check if there is an answer set
        std::set<std::string> signature = kb.GetSignature();

        // initialize program string:
        std::string program = "";

        // ---- set up program without query atom ---

        // add number of KB elements:
        program += NUM_KB_ELEMENTS + "(" + std::to_string(numFormulas) + ").";

        // add rules for each formula:
        program += handle_formulas_in_kb_mv(kb);

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_mv_MSS();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_mv_MSS();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_mv_MSS();

        program += add_general_rules_iterative_MSS();
        program += add_cs_rules_iterative_MSS_p_2();
        program += add_cs_sat_rules_iterative();
        program += add_cs_superset_rules_iterative();
        // program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative_MSS();

        std::set<std::string> formulas_in_MCS; // = kb.GetSignature();

        // std::set<std::string> remaining_atoms_tmp = kb.GetSignature();
        std::set<std::string> all_formulas;

        for (int i = 0; i < kb.size(); i++){
            std::string formula_name = FORMULA_PREFIX + std::to_string(i);
            all_formulas.insert(formula_name);
        }

        // while(!remaining_formulas.empty()){
        for (std::string f : all_formulas){

            bool is_in = formulas_in_MCS.find(f) != formulas_in_MCS.end();
            if (is_in){
                continue;
            }

            // // add query formulas:
            // std::string all_query_formulas = "";

            // for(auto f: all_formulas){
            //     std::string query_formula = QUERY_FORMULA + "(" + f + ").";
            //     all_query_formulas += query_formula;
            // }
            std::string query_formula = QUERY_FORMULA + "(" + f + ").";

            // add query atom to program:
            std::string curr_program = program + query_formula;

            // std::string curr_program = program + all_query_formulas;

            // ------------------------------------------------------------------

            auto encoding_end = std::chrono::steady_clock::now();
            double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                              / double (1000000.0);
            time_msrs::encoding_times.push_back(encoding_interval);

            time_msrs::num_solver_calls++;
            // Measure the time clingo takes to solve the program
            auto solver_start = std::chrono::steady_clock::now();
            
            // let Clingo solve the problem; check if an answer set exists:
            // also check which formulas are included in the MIS found
            std::set<std::string> formulas_in_curr_MCS = getFormulasInMCS(curr_program);

            // if (formulas_in_curr_MCS.empty()){
            //     break;
            // }

            for(auto a: formulas_in_curr_MCS){
                // std::cout << a << std::endl;
                formulas_in_MCS.insert(a);
                // remaining_formulas.erase(a);
            }

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

        }

        return (double)formulas_in_MCS.size();
    }  
}
