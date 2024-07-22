#include "mvpMUSBased.h"
#include <chrono>

using namespace constants;


double p_measure_MUS_based(Kb& kb){
    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        // std::cout << "Inconsistency value: 0" << std::endl;
        return 0;
    }

    else{
        std::set<std::string> signature = kb.GetSignature();

        // initialize program string:
        std::string program = "";

        // ---- set up program without query atom ---

        // At least one query formula must be included in at least one formula outside the CS:
        program += "validCS:-1{" + IN_CS + "(F):" + QUERY_FORMULA + "(F)}.";
        program += ":-not validCS.";

        // add number of KB elements:
        program += NUM_KB_ELEMENTS + "(" + std::to_string(numFormulas) + ").";

        // add rules for each formula:
        program += handle_formulas_in_kb_mv(kb);

        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_mv();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_mv();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_mv();

        program += add_general_rules_iterative();
        program += add_cs_rules_iterative_p();
        program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative();

        std::set<std::string> formulas_in_MUS;
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
            std::set<std::string> formulas_in_curr_MUS = getFormulasInMUS(curr_program);

            if (formulas_in_curr_MUS.empty()){
                break;
            }

            for(auto a: formulas_in_curr_MUS){
                formulas_in_MUS.insert(a);
                remaining_formulas.erase(a);
            }

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

        }

        return (double)formulas_in_MUS.size();
    }
}


double mv_measure_MUS_based(Kb& kb){

    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        // std::cout << "Inconsistency value: 0" << std::endl;
        return 0;
    }

    else{

        std::set<std::string> signature = kb.GetSignature();

        // initialize program string:
        std::string program = "";

        // ---- set up program without query atom ---

        // At least one query formula must be included in at least one formula outside the CS:
        program += "validCS:-1{" + IN_CS + "(F):" + QUERY_FORMULA + "(F)}.";
        program += ":-not validCS.";

        // add number of KB elements:
        program += NUM_KB_ELEMENTS + "(" + std::to_string(numFormulas) + ").";

        // add rules for each formula:
        program += handle_formulas_in_kb_mv(kb);

        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_mv();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_mv();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_mv();

        program += add_general_rules_iterative();
        program += add_cs_rules_iterative_p();
        program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative();

        std::set<std::string> formulas_in_MUS;
        std::set<std::string> atoms_in_MUS;

        // Create mapping between formula names and formulas
        std::map<std::string, Formula> formula_dict;
        std::vector<Formula> formulas = kb.GetFormulas();
        for (int i = 0; i < formulas.size(); i++){
            std::string formula_name = FORMULA_PREFIX + std::to_string(i);
            formula_dict.insert(std::pair<std::string, Formula>(formula_name, formulas.at(i)));
        }

        std::set<std::string> remaining_formulas;
        for(std::map<std::string,Formula>::iterator it = formula_dict.begin(); it != formula_dict.end(); ++it) {
            remaining_formulas.insert(it->first);
            // std::cout << it->first << std::endl;
        }

        // for (int i = 0; i < kb.size(); i++){
        //     std::string formula_name = FORMULA_PREFIX + std::to_string(i);
        //     remaining_formulas.insert(formula_name);
        // }

        while(!remaining_formulas.empty()){
            // add query formulas:
            std::string all_query_formulas = "";

            for(auto f: remaining_formulas){
                // std::cout << "f = " << f << std::endl;
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
            // also check which atoms are included in the MIS found
            std::set<std::string> formulas_in_curr_MUS = getFormulasInMUS(curr_program);

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

            if (formulas_in_curr_MUS.empty()){
                // std::cout << " -- No MCS found -- " << std::endl;
                break;
            }

            // std::cout << "formulas in current MCS:" << std::endl;
            for(auto f: formulas_in_curr_MUS){
                // std::cout << "\t" << f << std::endl;
                // get actual formula from dict
                Formula curr_formula = formula_dict.at(f);
                // std::cout << "\t" << curr_formula << std::endl;
                // get signature of formula
                std::set<std::string> curr_formula_signature = curr_formula.GetSignature();
                // add signature to atoms_in_MUS
                for (auto a: curr_formula_signature){
                    // std::cout << a << std::endl;
                    atoms_in_MUS.insert(a);
                }
                // remove formula from remaining_formulas
                // std::cout << f << " is being removed" << std::endl;
                remaining_formulas.erase(f);
            }

            // iterate over remaining_formulas:
            auto remaining_formulas_copy = remaining_formulas;
            for(auto f: remaining_formulas_copy){
                // - get signature of formula
                Formula curr_formula = formula_dict.at(f);
                std::set<std::string> curr_formula_signature = curr_formula.GetSignature();
                // - check if signature is subset of atoms_in_MUS

                // if(std::includes(curr_formula_signature.begin(), curr_formula_signature.end(), atoms_in_MUS.begin(), atoms_in_MUS.end())){
                if(std::includes(atoms_in_MUS.begin(), atoms_in_MUS.end(), curr_formula_signature.begin(), curr_formula_signature.end())){
                    //      -> if yes: remove formula from remaining_formulas
                    // std::cout << f << " is being removed: " << std::endl;
                    // std::cout << "\t" << curr_formula << std::endl;
                    remaining_formulas.erase(f);
                }
            }

        }

        return (double)atoms_in_MUS.size() / (double)signature.size();
    }

}
