#include "ForgettingBasedASP.h"
#include <chrono>
using namespace constants;

std::string getAtomOccRules(){
    // true:
    std::string atom_occ_rules = TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_T + "):-" + ATOM_OCC + "(X,Y)," + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_F + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_T + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_F + ").";
    // false:
    atom_occ_rules += TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_F + "):-" + ATOM_OCC + "(X,Y)," + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_T + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_F + ").";
    // forget_T:
    atom_occ_rules += TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_T + "):-" + ATOM_OCC + "(X,Y),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_F + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_F + ").";
    // forget_F:
    atom_occ_rules += TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_F + "):-" + ATOM_OCC + "(X,Y),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_T + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_F + "),not " + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_T + ").";

    // Evaluation of formulas which are atoms (forgetting operation):
    // true:
    atom_occ_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):-" + FORMULA_IS_ATOM_OCC + "(X,Y,Z)," + TRUTH_VALUE_PREDICATE_ATOM + "(Y,Z," + TRUTH_VALUE_T + ").";
    atom_occ_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):-" + FORMULA_IS_ATOM_OCC + "(X,Y,Z)," + TRUTH_VALUE_PREDICATE_ATOM + "(Y,Z," + TRUTH_VALUE_FORGET_T + ").";
    // false:
    atom_occ_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):-" + FORMULA_IS_ATOM_OCC + "(X,Y,Z)," + TRUTH_VALUE_PREDICATE_ATOM + "(Y,Z," + TRUTH_VALUE_F + ").";
    atom_occ_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):-" + FORMULA_IS_ATOM_OCC + "(X,Y,Z)," + TRUTH_VALUE_PREDICATE_ATOM + "(Y,Z," + TRUTH_VALUE_FORGET_F + ").";

    return atom_occ_rules;
}

std::string handle_formulas_in_kb_fb(Kb& kb, std::map<std::string, int>& atom_occurences){
    std::vector<Formula> formulas = kb.GetFormulas();
    std::string formula_rules = "";
    for (int i = 0; i < formulas.size(); i++){
        std::string formula_name = FORMULA_PREFIX + std::to_string(i);
        std::string kb_member_rule = KB_MEMBER + "(" + formula_name + ").";
        formula_rules += kb_member_rule;

        std::string curr_formula_rules = "";
        pl_to_ASP_fb(formulas.at(i), formula_name, curr_formula_rules, atom_occurences);
        formula_rules += curr_formula_rules;
    }
    return formula_rules;
}

void initialize_atom_occurences(std::map<std::string, int>& atom_occurences, Kb& kb){
    std::set<std::string> signature = kb.GetSignature();
    for(std::string atom : signature){
        std::transform(atom.begin(), atom.end(), atom.begin(),
            [](unsigned char c){ return std::tolower(c); });
        atom_occurences[atom] = 0;
    }
}

void pl_to_ASP_fb(Formula& formula, std::string formula_name, std::string& rules, std::map<std::string, int>& atom_occurences){
    if(formula.IsAtom()){
        // get atom name:
        std::string new_formula_name = formula.GetName();
        std::transform(new_formula_name.begin(), new_formula_name.end(), new_formula_name.begin(),
            [](unsigned char c){ return std::tolower(c); });
        // get label:
        int label = atom_occurences[new_formula_name];
        // update number of atom occurences:
        atom_occurences[new_formula_name]++;

        // add rule:
        rules += FORMULA_IS_ATOM_OCC + "(" + formula_name + "," + new_formula_name + "," + std::to_string(label) + ").";
        return;
    }

    if(formula.IsNegation()){
        auto subformulas = formula.GetSubformulas();
        Formula formula_without_negation = Formula(*(subformulas.begin()));
        std::string new_formula_name = formula_name + "_n";
        rules += NEGATION + "(" + new_formula_name + "," + formula_name + ").";
        pl_to_ASP_fb(formula_without_negation, new_formula_name, rules, atom_occurences);
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

            pl_to_ASP_fb(conjunct, new_formula_name, rules, atom_occurences);
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

            pl_to_ASP_fb(disjunct, new_formula_name, rules, atom_occurences);
        }
    }

    if(formula.IsImplication()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj = Formula(Type::OR, Formula(Type::NOT, left), right);
        pl_to_ASP_fb(disj, formula_name, rules, atom_occurences);
    }

    if(formula.IsEquivalence()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj_left = Formula(Type::OR, Formula(Type::NOT, left), right);
        Formula disj_right = Formula(Type::OR, Formula(Type::NOT, right), left);
        Formula conj = Formula(Type::AND, disj_left, disj_right);
        pl_to_ASP_fb(conj, formula_name, rules, atom_occurences);
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

std::string add_conjunction_rules_fb(){
    std::string conjunction_rules = "";
    // // false:
    // conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + "):" + CONJUNCTION + "(Y):-1{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):" + CONJUNCT_OF + "(X,Y)}.";
    // // conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + "):" + CONJUNCTION + "(Y):-1{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ")}," + CONJUNCT_OF + "(X,Y).";
    // // true:
    // conjunction_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):-" + CONJUNCTION + "(X),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ").";

    // T:
    // conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + "):" + CONJUNCTION + "(Y):-N{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(X,Y)}N," + NUM_CONJUNCTS + "(Y,N).";
    conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + "):" + CONJUNCTION + "(Y):-N{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(X,Y)}N," + NUM_CONJUNCTS + "(Y,N).";
    // F:
    // conjunction_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + ").";
    conjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(Y),not " + TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + ").";

    return conjunction_rules;
}

std::string add_disjunction_rules_fb(){
    std::string disjunction_rules = "";
    // // true:
    // disjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + "):" + DISJUNCTION + "(Y):-1{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):" + DISJUNCT_OF + "(X,Y)}.";
    // // false:
    // disjunction_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):-" + DISJUNCTION + "(X),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + ").";

    // F:
    disjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + "):" + DISJUNCTION + "(Y):-N{" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(X,Y)}N," + NUM_DISJUNCTS + "(Y,N).";
    // T:
    disjunction_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(Y),not " + TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + ").";

    return disjunction_rules;
}

std::string add_negation_rules_fb(){
    std::string negation_rules = "";
    // true:
    negation_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_T + "):-" + NEGATION + "(X,Y)," + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ").";
    // false:
    negation_rules += TRUTH_VALUE_PREDICATE + "(Y," + TRUTH_VALUE_F + "):-" + NEGATION + "(X,Y)," + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + ").";

    return negation_rules;
}

int forgetting_based_measure(Kb& kb){
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

        // add truth value facts:
        // "general" truth values
        program += "tv(" + TRUTH_VALUE_T + ";" + TRUTH_VALUE_F + ").";
        // truth values of atom occurences:
        // program += "atv(" + TRUTH_VALUE_T + ";" + TRUTH_VALUE_F + ";" + TRUTH_VALUE_FORGET_T + ";" + TRUTH_VALUE_FORGET_F + ").";

        // add integrity constraint:
        program += ":-" + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + ")," + KB_MEMBER + "(X).";

        // rules for extracting atom_occurence, atom and label:
        program += ATOM_OCC + "(X,Y):-" + FORMULA_IS_ATOM_OCC + "(_,X,Y).";
        program += ATOM + "(X):-" + ATOM_OCC + "(X,_).";
        // program += LABEL + "(X):-" + ATOM_OCC + "(_,X).";

        // unique atom evaluation:
        program += "1{" + TRUTH_VALUE_PREDICATE + "(X,Y):tv(Y)}1:-" + ATOM + "(X).";

        // an atom occurence can be true, false, or forgotten:
        // program += getAtomOccRules();

        // add rules for each formula:
        std::map<std::string, int> atom_occurences;
        initialize_atom_occurences(atom_occurences, kb);
        program += handle_formulas_in_kb_fb(kb, atom_occurences);

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_fb();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_fb();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_fb();

        // program += "atomOccForgotten(X,Y):-" + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_T + ").";
        // program += "atomOccForgotten(X,Y):-" + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_F + ").";

        // TEST:
        // {atomOccForgotten(A,L)} :- atomOcc(A,L).
        program += "{atomOccForgotten(A,L)}:-" + ATOM_OCC + "(A,L).";
        // truthValue(F,T) :- formulaIsAtomOcc(F,A,L), truthValue(A,T), not atomOccForgotten(A,L).
        // truthValue(F,t) :- formulaIsAtomOcc(F,A,L), truthValue(A,f), atomOccForgotten(A,L).
        // truthValue(F,f) :- formulaIsAtomOcc(F,A,L), truthValue(A,t), atomOccForgotten(A,L).
        program += TRUTH_VALUE_PREDICATE + "(F,T):-" + FORMULA_IS_ATOM_OCC + "(F,A,L)," + TRUTH_VALUE_PREDICATE + "(A,T),not atomOccForgotten(A,L).";
        program += TRUTH_VALUE_PREDICATE + "(F," + TRUTH_VALUE_T + "):-" + FORMULA_IS_ATOM_OCC + "(F,A,L)," + TRUTH_VALUE_PREDICATE + "(A," + TRUTH_VALUE_F + "),atomOccForgotten(A,L).";
        program += TRUTH_VALUE_PREDICATE + "(F," + TRUTH_VALUE_F + "):-" + FORMULA_IS_ATOM_OCC + "(F,A,L)," + TRUTH_VALUE_PREDICATE + "(A," + TRUTH_VALUE_T + "),atomOccForgotten(A,L).";
        // END TEST

        // minimize statement
        // program += "#minimize{1,X/Y:1{" + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_T + ")," + TRUTH_VALUE_PREDICATE_ATOM + "(X,Y," + TRUTH_VALUE_FORGET_F + ")}}.";
        program += "#minimize{1,X,Y:atomOccForgotten(X,Y)}.";

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

        return opt;
    }
}
