#include "ContensionASP.h"
#include <chrono>
using namespace constants;

std::string add_truth_values(){
    std::string all_truth_values = "tv(" + TRUTH_VALUE_T + ").";
    all_truth_values += "tv(" + TRUTH_VALUE_B + ").";
    all_truth_values += "tv(" + TRUTH_VALUE_F + ").";
    return all_truth_values;
}

void pl_to_ASP(Formula& formula, std::string formula_name, std::string& rules){

    if(formula.IsAtom()){
        std::string new_formula_name = formula.GetName();
        std::transform(new_formula_name.begin(), new_formula_name.end(), new_formula_name.begin(),
    [](unsigned char c){ return std::tolower(c); });
        rules += FORMULA_IS_ATOM + "(" + formula_name + "," + new_formula_name + ")." ;
        return;
    }

    if(formula.IsNegation()){
        auto subformulas = formula.GetSubformulas();
        Formula formula_without_negation = Formula(*(subformulas.begin()));
        std::string new_formula_name = formula_name + "_n";
        rules += NEGATION + "(" + new_formula_name + "," + formula_name + ").";
        pl_to_ASP(formula_without_negation, new_formula_name, rules);
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

            pl_to_ASP(conjunct, new_formula_name, rules);
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

            pl_to_ASP(disjunct, new_formula_name, rules);
        }
    }

    if(formula.IsImplication()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj = Formula(Type::OR, Formula(Type::NOT, left), right);
        pl_to_ASP(disj, formula_name, rules);
    }

    if(formula.IsEquivalence()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj_left = Formula(Type::OR, Formula(Type::NOT, left), right);
        Formula disj_right = Formula(Type::OR, Formula(Type::NOT, right), left);
        Formula conj = Formula(Type::AND, disj_left, disj_right);
        pl_to_ASP(conj, formula_name, rules);
    }

    if(formula.IsTautology()){
        rules += TRUTH_VALUE_PREDICATE + "(" + formula_name + "," + TRUTH_VALUE_T + ")." ;
        return;
    }

    if(formula.IsContradiction()){
        rules += TRUTH_VALUE_PREDICATE + "(" + formula_name + "," + TRUTH_VALUE_F + ")." ;
        return;
    }


    return;
}

std::string handle_formulas_in_kb(Kb& kb){
    std::vector<Formula> formulas = kb.GetFormulas();
    std::string formula_rules = "";
    for (int i = 0; i < formulas.size(); i++){
        std::string formula_name = FORMULA_PREFIX + std::to_string(i);
        std::string kb_member_rule = KB_MEMBER + "(" + formula_name + ").";
        formula_rules += kb_member_rule;

        std::string curr_formula_rules = "";
        pl_to_ASP(formulas.at(i), formula_name, curr_formula_rules);
        formula_rules += curr_formula_rules;
    }

    return formula_rules;
}

std::string add_conjunction_rules(){
    std::string conjunction_rules = "";
    // T:
    conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + "):" + CONJUNCTION + "(Y):-N{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(X,Y)}N," + NUM_CONJUNCTS + "(Y,N).";
    // F:
    conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + "):" + CONJUNCTION + "(Y):-1{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ")}," + CONJUNCT_OF + "(X,Y).";
    // B:
    conjunction_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_B + "):-" + CONJUNCTION + "(X),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ")." ;

    return conjunction_rules;
}

std::string add_disjunction_rules(){
    std::string disjunction_rules = "";
    // T:
    disjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + "):" + DISJUNCTION + "(Y):-1{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + ")}," + DISJUNCT_OF + "(X,Y).";
    // F:
    disjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + "):" + DISJUNCTION + "(Y):-N{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(X,Y)}N," + NUM_DISJUNCTS + "(Y,N).";
    // B:
    disjunction_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_B + "):-" + DISJUNCTION + "(X),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ")." ;

    return disjunction_rules;
}

std::string add_negation_rules(){
    std::string negation_rules = "";
    // T:
    negation_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + "):-" + NEGATION + "(X,Y)," + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ").";
    // F:
    negation_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + "):-" + NEGATION + "(X,Y)," + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + ").";
    // B:
    negation_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_B + "):-" + NEGATION + "(X,Y)," + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_B + ").";

    return negation_rules;
}


int contension_measure(Kb& kb){
    // Parse file:
    // Parser p = Parser();
    // Kb kb = p.ParseKbFromFile(path_to_kb);

    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(kb.size() == 0){
        // std::cout << "Inconsistency value: 0" << std::endl; 
        return 0;
    }

    else{
        // initialize program string:
        std::string program = "";

        // add truth value rules:
        program += add_truth_values();

        // add integrity constraint:
        program += ":-" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ")," + KB_MEMBER + "(X).";

        // Unique atom evaluation:
        program += "1{" + TRUTH_VALUE_PREDICATE + "(X,Y):tv(Y)}1:-" + ATOM + "(X).";

        // add rule for each atom in the signature:
        program += add_atom_rules(kb);

        // add rules for each formula:
        program += handle_formulas_in_kb(kb);

        // add universal rules for connectors and formulas consisting of single atoms:
        program += TRUTH_VALUE_PREDICATE + "(X,Z):tv(Z):-" + FORMULA_IS_ATOM + "(X,Y)," + TRUTH_VALUE_PREDICATE + "(Y,Z).";
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules();

        // add minimize statement:
        program += "#minimize{1,X:" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_B + ")," + ATOM + "(X)}.";

        // std::cout << program << std::endl;
        auto encoding_end = std::chrono::steady_clock::now();
        double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                          / double (1000000.0);
        time_msrs::encoding_times.push_back(encoding_interval);

        time_msrs::num_solver_calls++;
        // Measure the time clingo takes to solve the program
        auto solver_start = std::chrono::steady_clock::now();
        // let Clingo solve the problem; retrieve optimum:
        int opt = compute_optimum(program);
        auto solver_end = std::chrono::steady_clock::now();
        double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                          / double (1000000.0);
        time_msrs::solver_times_total.push_back(solver_interval);
        // std::cout << "Inconsistency value: " << opt << std::endl;
        return opt;
    }
}
