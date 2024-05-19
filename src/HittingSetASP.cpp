#include "HittingSetASP.h"
#include <chrono>
using namespace constants;

std::string ensureKbMemberToBeTrue(){
    // true:
    std::string kb_member_rules = TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + ")," + INTERPRETATION + "(Y)," + KB_MEMBER + "(X)," + INTERPRETATION_REQUIRED + "(Y).";
    // false:
    kb_member_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):-" + KB_MEMBER + "(X),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + ").";

    return kb_member_rules;
}

std::string handle_formulas_in_kb_hs(Kb& kb){
    std::vector<Formula> formulas = kb.GetFormulas();
    std::string formula_rules = "";
    for (int i = 0; i < formulas.size(); i++){
        std::string formula_name = FORMULA_PREFIX + std::to_string(i);
        std::string kb_member_rule = KB_MEMBER + "(" + formula_name + ").";
        formula_rules += kb_member_rule;

        std::string curr_formula_rules = "";
        pl_to_ASP_hs(formulas.at(i), formula_name, curr_formula_rules, formulas.size());
        formula_rules += curr_formula_rules;
    }

    return formula_rules;
}

void pl_to_ASP_hs(Formula& formula, std::string formula_name, std::string& rules, int num_formulas){
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
        pl_to_ASP_hs(formula_without_negation, new_formula_name, rules, num_formulas);
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

            pl_to_ASP_hs(conjunct, new_formula_name, rules, num_formulas);
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

            pl_to_ASP_hs(disjunct, new_formula_name, rules, num_formulas);
        }
    }

    if(formula.IsImplication()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj = Formula(Type::OR, Formula(Type::NOT, left), right);
        pl_to_ASP_hs(disj, formula_name, rules, num_formulas);
    }

    if(formula.IsEquivalence()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj_left = Formula(Type::OR, Formula(Type::NOT, left), right);
        Formula disj_right = Formula(Type::OR, Formula(Type::NOT, right), left);
        Formula conj = Formula(Type::AND, disj_left, disj_right);
        pl_to_ASP_hs(conj, formula_name, rules, num_formulas);
    }

    if(formula.IsTautology()){
        rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(" + formula_name + ",1.." + std::to_string(num_formulas) + "," + TRUTH_VALUE_T + ")." ;
        return;
    }

    if(formula.IsContradiction()){
        rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(" + formula_name + ",1.." + std::to_string(num_formulas) + "," + TRUTH_VALUE_F + ")." ;
        return;
    }

    return;
}

std::string add_conjunction_rules_hs(){
    std::string conjunction_rules = "";

    // true:
    conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + CONJUNCTION + "(X)," + INTERPRETATION + "(Y),N{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(Z,X)}N," + NUM_CONJUNCTS + "(X,N).";

    // false:
    conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + ").";

    return conjunction_rules;
}

std::string add_disjunction_rules_hs(){
    std::string disjunction_rules = "";

    // false:
    disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + DISJUNCTION + "(X)," + INTERPRETATION + "(Y),N{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(Z,X)}N," + NUM_DISJUNCTS + "(X,N).";
    // true:
    disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + ").";

    return disjunction_rules;
}

std::string add_negation_rules_hs(){
    std::string negation_rules = "";

    // true:
    negation_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_F + ").";
    // false:
    negation_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_T + ").";

    return negation_rules;
}

int hitting_set_measure(Kb& kb){

    // Parse file:
    // Parser p = Parser();
    // Kb kb = p.ParseKbFromFile(path_to_kb);

    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        // std::cout << "Inconsistency value: 0" << std::endl;
        return 0;
    }

    else{
        // initialize program string:
        std::string program = "";

        // add truth value facts:
        program += "tv(" + TRUTH_VALUE_T + ";" + TRUTH_VALUE_F + ").";

        // add interpretation facts:
        program += INTERPRETATION + "(1.." + std::to_string(numFormulas) + ").";

        // add atom facts:
        program += add_atom_rules(kb);

        // add integrity constraint:
        program += ":-" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ")," + KB_MEMBER + "(X).";

        program += "1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,Z):tv(Z)}1:-" + ATOM + "(X)," + INTERPRETATION + "(Y).";

        // every formula in the KB must be true wrt. at least one interpretation:
        program += ensureKbMemberToBeTrue();

        // add rules for each formula:
        program += handle_formulas_in_kb_hs(kb);

        // if a formula is in fact an atom:
        program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,Z):tv(Z):-" + FORMULA_IS_ATOM + "(X,W)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(W,Y,Z)," + INTERPRETATION + "(Y).";

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_hs();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_hs();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_hs();

        // Cardinality Constraint:
        program += "1{" + INTERPRETATION_REQUIRED + "(X):" + INTERPRETATION + "(X)}" + std::to_string(numFormulas) + ".";

        // avoid symmetries:
        program += ":-" + INTERPRETATION_REQUIRED + "(I),1<I,not " + INTERPRETATION_REQUIRED + "(I-1).";

        program += "#minimize{1,X:" + INTERPRETATION_REQUIRED + "(X)}.";

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


        if(opt == -1){
            return opt;
        }
        else{
            return opt - 1;
        }
    }
}
