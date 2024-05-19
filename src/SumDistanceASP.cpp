#include "SumDistanceASP.h"
#include <chrono>
using namespace constants;

std::string handle_formulas_in_kb_d_sum(Kb& kb){
    std::vector<Formula> formulas = kb.GetFormulas();
    std::string formula_rules = "";
    for (int i = 0; i < formulas.size(); i++){
        std::string formula_name = FORMULA_PREFIX + std::to_string(i);
        std::string kb_member_rule = KB_MEMBER + "(" + formula_name + "," + std::to_string(i) +  ").";
        formula_rules += kb_member_rule;

        std::string curr_formula_rules = "";
        pl_to_ASP_d_sum(formulas.at(i), formula_name, curr_formula_rules, formulas.size());
        formula_rules += curr_formula_rules;
    }

    return formula_rules;
}


void pl_to_ASP_d_sum(Formula& formula, std::string formula_name, std::string& rules, int num_formulas){
    if(formula.IsAtom()){
        std::string new_formula_name = formula.GetName();
        std::transform(new_formula_name.begin(), new_formula_name.end(), new_formula_name.begin(),
            [](unsigned char c){ return std::tolower(c); });
        rules += FORMULA_IS_ATOM + "(" + formula_name + "," + new_formula_name + ").";
        return;
    }

    if(formula.IsNegation()){
        auto subformulas = formula.GetSubformulas();
        Formula formula_without_negation = Formula(*(subformulas.begin()));
        std::string new_formula_name = formula_name + "_n";
        rules += NEGATION + "(" + new_formula_name + "," + formula_name + ").";
        pl_to_ASP_d_sum(formula_without_negation, new_formula_name, rules, num_formulas);
    }

    if(formula.IsConjunction()){
        rules += CONJUNCTION + "(" + formula_name + ").";
        std::vector<Formula> conjuncts = formula.GetSubformulas();
        rules += NUM_CONJUNCTS + "(" + formula_name + "," + std::to_string(conjuncts.size()) + ").";

        int conjunct_count = 0;
        for(auto conjunct : conjuncts){
            std::string new_formula_name = formula_name + "_" + std::to_string(conjunct_count);
            rules += CONJUNCT_OF + "(" + new_formula_name + "," + formula_name + ").";

            conjunct_count++;

            pl_to_ASP_d_sum(conjunct, new_formula_name, rules, num_formulas);
        }
    }

    if(formula.IsDisjunction()){
        rules += DISJUNCTION + "(" + formula_name + ").";
        std::vector<Formula> disjuncts = formula.GetSubformulas();
        rules += NUM_DISJUNCTS + "(" + formula_name + "," + std::to_string(disjuncts.size()) + ").";

        int disjunct_count = 0;
        for(auto disjunct : disjuncts){
            std::string new_formula_name = formula_name + "_" + std::to_string(disjunct_count);
            rules += DISJUNCT_OF + "(" + new_formula_name + "," + formula_name + ").";

            disjunct_count++;

            pl_to_ASP_d_sum(disjunct, new_formula_name, rules, num_formulas);
        }
    }

    if(formula.IsImplication()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj = Formula(Type::OR, Formula(Type::NOT, left), right);
        pl_to_ASP_d_sum(disj, formula_name, rules, num_formulas);
    }

    if(formula.IsEquivalence()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj_left = Formula(Type::OR, Formula(Type::NOT, left), right);
        Formula disj_right = Formula(Type::OR, Formula(Type::NOT, right), left);
        Formula conj = Formula(Type::AND, disj_left, disj_right);
        pl_to_ASP_d_sum(conj, formula_name, rules, num_formulas);
    }

    if(formula.IsTautology()){
        rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(" + formula_name + ",0.." + std::to_string(num_formulas) + "," + TRUTH_VALUE_T + ")." ;
        return;
    }

    if(formula.IsContradiction()){
        rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(" + formula_name + ",0.." + std::to_string(num_formulas) + "," + TRUTH_VALUE_F + ")." ;
        return;
    }

    return;
}

int sum_distance_measure(Kb& kb){

    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        return 0;
    }

    else{
        // initialize program string:
        std::string program = "";

        // add truth value facts:
        program += "tv(" + TRUTH_VALUE_T + ";" + TRUTH_VALUE_F + ").";

        // add interpretation facts:
        program += INTERPRETATION + "(0.." + std::to_string(numFormulas) + ").";

        // add atom facts:
        program += add_atom_rules(kb);

        // unique atom evaluation:
        program += "1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,I,T):tv(T)}1:-" + ATOM + "(A)," + INTERPRETATION + "(I).";

        // add KB members:
        // program += KB_MEMBER + "(F):-" + KB_MEMBER + "(F,_).";
        program += handle_formulas_in_kb_d_sum(kb);

        // ensure that the x-th interpretation satisfies the x-th formula in the KB:
        // program += add_kb_member_rules_dalal();

        // integrity constraint:
        program += ":-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,I," + TRUTH_VALUE_F + ")," + KB_MEMBER + "(F,I).";

        // if a formula is in fact an atom:
        program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,I,T):tv(T):-" + FORMULA_IS_ATOM + "(F,A)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,I,T)," + INTERPRETATION + "(I).";

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_dalal();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_dalal();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_dalal();

        // Define Dalal distance between two interpretations:
        // program += add_dalal_distance_rules();

        // Summation:
        // program += DALAL_SUM + "(X):-X=#sum{Y,I:" + DALAL + "(I," + std::to_string(numFormulas) + ",Y)," + INTERPRETATION + "(I)},X>=0.";

        // Minimization:
        // program += "#minimize{X:" + DALAL_SUM + "(X)}.";
        program += "#minimize{1,A,I:" + ATOM + "(A)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,I,T),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A," + std::to_string(numFormulas) + ",T)}.";
        
        // std::cout << program << std::endl;
        auto encoding_end = std::chrono::steady_clock::now();
        double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                          / double (1000000.0);
        time_msrs::encoding_times.push_back(encoding_interval);

        time_msrs::num_solver_calls++;
        // Measure the time clingo takes to solve the program
        auto solver_start = std::chrono::steady_clock::now();
        // let Clingo solve the problem; retrieve optimum:
        int opt = compute_optimum_with_inf(program);
        auto solver_end = std::chrono::steady_clock::now();
        double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                          / double (1000000.0);
        time_msrs::solver_times_total.push_back(solver_interval);

        // std::cout << "Optimum Dalal: " << opt << std::endl;

        return opt;
    }
}
