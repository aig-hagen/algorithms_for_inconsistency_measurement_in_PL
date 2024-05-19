#include "LTLMeasuresASP.h"
using namespace constants;

// Author: Isabelle Kuhlmann
/*
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
*/
std::string add_atom_rules_formula(Formula f){
    std::set<std::string> signature = f.GetSignature();
    std::string atom_rules = "";
    for(std::string atom : signature){
        std::transform(atom.begin(), atom.end(), atom.begin(),
            [](unsigned char c){ return std::tolower(c); });
        std::string atom_rule = ATOM + "(" + atom + ").";
        atom_rules += atom_rule;
    } 
    return atom_rules;
}

std::string add_truth_values_LTL(){
    std::string all_truth_values = "tv(" + TRUTH_VALUE_T + ").";
    all_truth_values += "tv(" + TRUTH_VALUE_B + ").";
    all_truth_values += "tv(" + TRUTH_VALUE_F + ").";
    return all_truth_values;
}

std::string handle_formulas_in_kb_LTL(Kb& kb){
    std::vector<Formula> formulas = kb.GetFormulas();
    std::string formula_rules = "";
    for (int i = 0; i < formulas.size(); i++){
        std::string formula_name = FORMULA_PREFIX + std::to_string(i);
        std::string kb_member_rule = KB_MEMBER + "(" + formula_name + ").";
        formula_rules += kb_member_rule;

        std::string curr_formula_rules = "";
        ltl_to_asp(formulas.at(i), formula_name, curr_formula_rules);
        formula_rules += curr_formula_rules;
    }

    return formula_rules;
}

std::string handle_single_formula_in_kb_LTL(Formula f){
    // TODO: add argument (std::string) that allows for individual formula names 
    std::string formula_rules = "";
    std::string formula_name = FORMULA_PREFIX;
    std::string kb_member_rule = KB_MEMBER + "(" + formula_name + ").";
    formula_rules += kb_member_rule;

    std::string curr_formula_rules = "";
    ltl_to_asp(f, formula_name, curr_formula_rules);
    formula_rules += curr_formula_rules;
    
    return formula_rules;
}

std::string add_conjunction_rules_LTL(){
    std::string conjunction_rules = "";
    // T:
    conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y,S," + TRUTH_VALUE_T + "):-" + CONJUNCTION + "(Y)," + IS_STATE + "(S),N{" + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(X,Y)}N," + NUM_CONJUNCTS + "(Y,N).";
    // F:
    conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y,S," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(Y)," + IS_STATE + "(S),1{" + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_F + ")}," + CONJUNCT_OF + "(X,Y).";
    // B:
    conjunction_rules += TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_B + "):-" + CONJUNCTION + "(X)," + IS_STATE + "(S),not " + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_F + ")." ;

    return conjunction_rules;
}

std::string add_disjunction_rules_LTL(){
    std::string disjunction_rules = "";
    // T:
    disjunction_rules += TRUTH_VALUE_PREDICATE + "(Y,S," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(Y)," + IS_STATE + "(S),1{" + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_T + ")}," + DISJUNCT_OF + "(X,Y).";
    // F:
    disjunction_rules += TRUTH_VALUE_PREDICATE + "(Y,S," + TRUTH_VALUE_F + "):-" + DISJUNCTION + "(Y)," + IS_STATE + "(S),N{" + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(X,Y)}N," + NUM_DISJUNCTS + "(Y,N).";
    // B:
    disjunction_rules += TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_B + "):-" + DISJUNCTION + "(X)," + IS_STATE + "(S),not " + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_F + ")." ;

    return disjunction_rules;
}

std::string add_negation_rules_LTL(){
    std::string negation_rules = "";
    // T:
    negation_rules += TRUTH_VALUE_PREDICATE + "(Y,S," + TRUTH_VALUE_T + "):-" + NEGATION + "(X,Y)," + IS_STATE + "(S)," + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_F + ").";
    // F:
    negation_rules += TRUTH_VALUE_PREDICATE + "(Y,S," + TRUTH_VALUE_F + "):-" + NEGATION + "(X,Y)," + IS_STATE + "(S)," + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_T + ").";
    // B:
    negation_rules += TRUTH_VALUE_PREDICATE + "(Y,S," + TRUTH_VALUE_B + "):-" + NEGATION + "(X,Y)," + IS_STATE + "(S)," + TRUTH_VALUE_PREDICATE + "(X,S," + TRUTH_VALUE_B + ").";

    return negation_rules;
}

std::string add_next_rules(){
    std::string next_rules = "";
    // i < m:
    next_rules += TRUTH_VALUE_PREDICATE + "(F,S1,T):-" + IS_NEXT + "(F,G)," + IS_STATE + "(S1),tv(T),S2=S1+1,S1<M," + FINAL_STATE + "(M)," + TRUTH_VALUE_PREDICATE + "(G,S2,T).";
    // i >= m:
    next_rules += TRUTH_VALUE_PREDICATE + "(F,S," + TRUTH_VALUE_F + "):-" + IS_NEXT + "(F,_)," + IS_STATE + "(S),S>=M," + FINAL_STATE + "(M).";
    return next_rules;
}

std::string add_until_rules(){
    std::string until_rules = "";
    // T: 
    until_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_T + "):-" + IS_UNTIL + "(F,G,H)," + IS_STATE + "(S1)," + IS_STATE + "(S2),S2>S1,S2<=M," + FINAL_STATE + "(M),X{"
        + TRUTH_VALUE_PREDICATE + "(G,S," + TRUTH_VALUE_T + "):" + IS_STATE + "(S),S>=S1,S<S2}X,X=S2-S1," + TRUTH_VALUE_PREDICATE + "(H,S2," + TRUTH_VALUE_T + ").";
    // B:
    until_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_B + "):-" + IS_UNTIL + "(F,G,H)," + IS_STATE + "(S1)," + IS_STATE + "(S2),S2>S1,S2<=M," + FINAL_STATE + "(M),X{"
        + TRUTH_VALUE_PREDICATE + "(G,S," + TRUTH_VALUE_B + "):" + IS_STATE + "(S),S>=S1,S<S2;" + TRUTH_VALUE_PREDICATE + "(H,S2," + TRUTH_VALUE_B + ");" + TRUTH_VALUE_PREDICATE + "(G,S," + TRUTH_VALUE_T + "):"
        + IS_STATE + "(S),S>=S1,S<S2;" + TRUTH_VALUE_PREDICATE + "(H,S2," + TRUTH_VALUE_T + ")}X,not " + TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_T + "),X=S2-S1+1.";
    // F:
    until_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_F + "):-" + IS_UNTIL + "(F,_,_)," + IS_STATE  + "(S1),not " + TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_B + ").";

    return until_rules;
}

std::string add_globally_rules(){
    std::string gobally_rules = "";
    // T:
    gobally_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_T + "):-" + IS_GLOBALLY + "(F,G)," + IS_STATE + "(S1),X{" + TRUTH_VALUE_PREDICATE + "(G,S," + TRUTH_VALUE_T + "):" + IS_STATE + "(S),S>S1}X," + FINAL_STATE + "(M),X=M-S1.";
    // F:
    gobally_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_F + "):-" + IS_GLOBALLY + "(F,G)," + IS_STATE + "(S1),1{" + TRUTH_VALUE_PREDICATE + "(G,S," + TRUTH_VALUE_F + "):" + IS_STATE + "(S),S>S1}.";
    // B:
    gobally_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_B + "):-" + IS_GLOBALLY + "(F,_)," + IS_STATE + "(S1),not " + TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_F + ").";

    return gobally_rules;
}

std::string add_finally_rules(){
    std::string finally_rules = "";
    // T:
    finally_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_T + "):-" + IS_FINALLY + "(F,G)," + IS_STATE + "(S1)," + TRUTH_VALUE_PREDICATE + "(G,S2," + TRUTH_VALUE_T + ")," + IS_STATE + "(S2),S2>S1.";
    // F:
    finally_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_F + "):-" + IS_FINALLY + "(F,G)," + IS_STATE + "(S1),X{" + TRUTH_VALUE_PREDICATE + "(G,S," + TRUTH_VALUE_F + "):" + IS_STATE + "(S),S>S1}X," + FINAL_STATE + "(M),X=M-S1.";
    // B:
    finally_rules += TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_B + "):-" + IS_FINALLY + "(F,_)," + IS_STATE + "(S1),not " + TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE + "(F,S1," + TRUTH_VALUE_F + ").";

    return finally_rules;
}

void ltl_to_asp(Formula& formula, std::string formula_name, std::string& rules){
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
        ltl_to_asp(formula_without_negation, new_formula_name, rules);
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

            ltl_to_asp(conjunct, new_formula_name, rules);
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

            ltl_to_asp(disjunct, new_formula_name, rules);
        }
    }

    if(formula.IsImplication()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj = Formula(Type::OR, Formula(Type::NOT, left), right);
        ltl_to_asp(disj, formula_name, rules);
    }

    if(formula.IsEquivalence()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj_left = Formula(Type::OR, Formula(Type::NOT, left), right);
        Formula disj_right = Formula(Type::OR, Formula(Type::NOT, right), left);
        Formula conj = Formula(Type::AND, disj_left, disj_right);
        ltl_to_asp(conj, formula_name, rules);
    }

    if(formula.IsNext()){
        auto subformulas = formula.GetSubformulas();
        Formula base_formula = Formula(*(subformulas.begin()));
        std::string new_formula_name = formula_name + "_x";
        rules += IS_NEXT + "(" + formula_name + "," + new_formula_name + ").";
        ltl_to_asp(base_formula, new_formula_name, rules);
    }

    if(formula.IsUntil()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);

        std::string new_formula_name_left = formula_name + "_l";
        std::string new_formula_name_right = formula_name + "_r";
        rules += IS_UNTIL + "(" + formula_name + "," + new_formula_name_left + "," + new_formula_name_right + ").";
        ltl_to_asp(left, new_formula_name_left, rules);
        ltl_to_asp(right, new_formula_name_right, rules);
    }

    if(formula.IsGlobally()){
        auto subformulas = formula.GetSubformulas();
        Formula base_formula = Formula(*(subformulas.begin()));
        std::string new_formula_name = formula_name + "_g";
        rules += IS_GLOBALLY + "(" + formula_name + "," + new_formula_name + ").";
        ltl_to_asp(base_formula, new_formula_name, rules);
    }

    if(formula.IsFinally()){
        auto subformulas = formula.GetSubformulas();
        Formula base_formula = Formula(*(subformulas.begin()));
        std::string new_formula_name = formula_name + "_f";
        rules += IS_FINALLY + "(" + formula_name + "," + new_formula_name + ").";
        ltl_to_asp(base_formula, new_formula_name, rules);
    }
}

std::string get_base_program_LTL(Kb& kb){

    std::string program = "";

    program += IS_STATE + "(0..X):-" + FINAL_STATE + "(X).";

    // add truth value rules:
    program += add_truth_values_LTL();

    // add integrity constraint:
    program += ":-" + TRUTH_VALUE_PREDICATE + "(X,0," + TRUTH_VALUE_F + ")," + KB_MEMBER + "(X)," + IS_STATE + "(0).";

    // Unique atom evaluation:
    program += "1{" + TRUTH_VALUE_PREDICATE + "(A,S,T):tv(T)}1:-" + ATOM + "(A)," + IS_STATE + "(S).";

    // add rule for each atom in the signature:
    program += add_atom_rules(kb);

    // add rules for each formula:
    program += handle_formulas_in_kb_LTL(kb);

    // add universal rules for connectors and formulas consisting of single atoms:
    program += TRUTH_VALUE_PREDICATE + "(F,S,T):-tv(T)," + FORMULA_IS_ATOM + "(F,A)," + IS_STATE + "(S)," + TRUTH_VALUE_PREDICATE + "(A,S,T).";

    if(program.find(CONJUNCTION) != std::string::npos)
        program += add_conjunction_rules_LTL();
    if(program.find(DISJUNCTION) != std::string::npos)
        program += add_disjunction_rules_LTL();
    if(program.find(NEGATION) != std::string::npos)
        program += add_negation_rules_LTL();
    if(program.find(IS_NEXT) != std::string::npos)
        program += add_next_rules();
    if(program.find(IS_UNTIL) != std::string::npos)
        program += add_until_rules();
    if(program.find(IS_GLOBALLY) != std::string::npos)
        program += add_globally_rules();
    if(program.find(IS_FINALLY) != std::string::npos)
        program += add_finally_rules();

    return program;
}

int contension_measure_LTL(Kb& kb, int m){

    // Check if KB is empty:
    if(kb.size() == 0){
        return 0;
    }

    else{

        std::vector<Formula> formulas = kb.GetFormulas();

        // initialize program string:
        std::string program = "";

        // set number of states:
        program += FINAL_STATE + "(" + std::to_string(m) + ").";

        program += get_base_program_LTL(kb);

        // compute inconsistency value:
        program += NUM_B_IN_STATE + "(S,X):-" + IS_STATE + "(S),#count{A:" + TRUTH_VALUE_PREDICATE + "(A,S," + TRUTH_VALUE_B + ")," + ATOM + "(A)}=X.";
        
        program += SUM_B + "(X):-#sum{Y,S:" + NUM_B_IN_STATE + "(S,Y)," + IS_STATE + "(S)}=X.";
        
        program += "#minimize{X:" + SUM_B + "(X)}.";

        // let Clingo solve the problem; retrieve optimum:
        int opt = compute_optimum_with_inf(program);
        return opt;
    }

    return 0;
}

int drastic_measure_LTL(Kb& kb, int m){

    // Check if KB is empty:
    if(kb.size() == 0){
        return 0;
    }

    else{

        std::vector<Formula> formulas = kb.GetFormulas();

        // initialize program string:
        std::string program = "";

        // set number of states:
        program += FINAL_STATE + "(" + std::to_string(m) + ").";

        program += get_base_program_LTL(kb);

        // compute inconsistency value:
        program += AFFECTED_STATE + "(S):-" + IS_STATE + "(S)," + TRUTH_VALUE_PREDICATE + "(A,S," + TRUTH_VALUE_B + ")," + ATOM + "(A).";
        
        program += "#minimize{1,S:" + AFFECTED_STATE + "(S)}.";

        // let Clingo solve the problem; retrieve optimum:
        int opt = compute_optimum_with_inf(program);
        return opt;
    }

    return 0;
}