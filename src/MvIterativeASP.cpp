#include "MvIterativeASP.h"
#include "InconsistencyMeasureASP.h"
#include <chrono>
using namespace constants;


// std::string ensureKbMemberToBeTrue(){
//     // true:
//     std::string kb_member_rules = TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + "):-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + ")," + INTERPRETATION + "(Y)," + KB_MEMBER + "(X)," + INTERPRETATION_REQUIRED + "(Y).";
//     // false:
//     kb_member_rules += TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_F + "):-" + KB_MEMBER + "(X),not " + TRUTH_VALUE_PREDICATE + "(X," + TRUTH_VALUE_T + ").";

//     return kb_member_rules;
// }

std::string handle_formulas_in_kb_mv(Kb& kb){
    std::vector<Formula> formulas = kb.GetFormulas();
    std::string formula_rules = "";
    for (int i = 0; i < formulas.size(); i++){

        // std::cout << "\tCurrent formula: " << formulas.at(i) << std::endl;

        std::string formula_name = FORMULA_PREFIX + std::to_string(i);
        for(std::string at : formulas.at(i).GetSignature()){
            std::transform(at.begin(), at.end(), at.begin(),
                [](unsigned char c){ return std::tolower(c); });
            std::string atom_in_formula_rule = ATOM_IN_FORMULA_MV + "(" + at + "," + formula_name + ").";
            formula_rules += atom_in_formula_rule;
        }

        std::string curr_formula_rules = "";
        pl_to_ASP_mv(formulas.at(i), formula_name, curr_formula_rules, formulas.size());
        formula_rules += curr_formula_rules;
    }

    return formula_rules;
}

// std::string handle_formulas_in_kb_mv_new(Kb& kb, std::set<std::string> atoms_in_MUS){

//     std::vector<Formula> formulas = kb.GetFormulas();
//     std::string formula_rules = "";
//     for (int i = 0; i < formulas.size(); i++){

//         std::cout << "\tCurrent formula: " << formulas.at(i) << std::endl;

//         std::string formula_name = FORMULA_PREFIX + std::to_string(i);

//         std::set<std::string> formula_signature = formulas.at(i).GetSignature();
//         std::set<std::string> formula_signature_lowercase;
//         for(std::string at : formula_signature){
//             std::transform(at.begin(), at.end(), at.begin(),
//                 [](unsigned char c){ return std::tolower(c); });
//             formula_signature_lowercase.insert(at);
//         }

//         if (std::includes(atoms_in_MUS.begin(), atoms_in_MUS.end(), formula_signature_lowercase.begin(), formula_signature_lowercase.end())){
//             std::cout << "Excluding " << formulas.at(i) << " at index " << i << std::endl;
//             continue;
//         }
//         else{
//             for(std::string at : formula_signature_lowercase){
//                 // std::transform(at.begin(), at.end(), at.begin(),
//                 //     [](unsigned char c){ return std::tolower(c); });
//                 std::string atom_in_formula_rule = ATOM_IN_FORMULA + "(" + at + "," + formula_name + ").";
//                 formula_rules += atom_in_formula_rule;
//             }

//             std::string curr_formula_rules = "";
//             pl_to_ASP_mv(formulas.at(i), formula_name, curr_formula_rules, formulas.size());
//             formula_rules += curr_formula_rules;
//         }
//     }

//     return formula_rules;
// }

void pl_to_ASP_mv(Formula& formula, std::string formula_name, std::string& rules, int num_formulas){
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
        pl_to_ASP_mv(formula_without_negation, new_formula_name, rules, num_formulas);
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

            pl_to_ASP_mv(conjunct, new_formula_name, rules, num_formulas);
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

            pl_to_ASP_mv(disjunct, new_formula_name, rules, num_formulas);
        }
    }

    if(formula.IsImplication()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj = Formula(Type::OR, Formula(Type::NOT, left), right);
        pl_to_ASP_mv(disj, formula_name, rules, num_formulas);
    }

    if(formula.IsEquivalence()){
        auto subformulas = formula.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj_left = Formula(Type::OR, Formula(Type::NOT, left), right);
        Formula disj_right = Formula(Type::OR, Formula(Type::NOT, right), left);
        Formula conj = Formula(Type::AND, disj_left, disj_right);
        pl_to_ASP_mv(conj, formula_name, rules, num_formulas);
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

// for iterative version:
std::string add_conjunction_rules_mv(){
    std::string conjunction_rules = "";

    // true:
    conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + CONJUNCTION + "(X)," + INTERPRETATION + "(Y),N{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(Z,X)}N," + NUM_CONJUNCTS + "(X,N).";

    // false:
    // conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + ").";
    conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X)," + INTERPRETATION + "(Y),1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_F + "):" + CONJUNCT_OF + "(Z,X)}."; // N," + NUM_CONJUNCTS + "(X,N).";

    return conjunction_rules;
}

std::string add_disjunction_rules_mv(){
    std::string disjunction_rules = "";

    // false:
    disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + DISJUNCTION + "(X)," + INTERPRETATION + "(Y),N{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(Z,X)}N," + NUM_DISJUNCTS + "(X,N).";
    // true:
    // disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + ").";
    disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X)," + INTERPRETATION + "(Y),1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_T + "):" + DISJUNCT_OF + "(Z,X)}."; // N," + NUM_DISJUNCTS + "(X,N).";

    return disjunction_rules;
}

std::string add_negation_rules_mv(){
    std::string negation_rules = "";

    // true:
    negation_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_F + ").";
    // false:
    negation_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y," + TRUTH_VALUE_T + ").";

    return negation_rules;
}

// for iterative MSS/MCS version:
std::string add_conjunction_rules_mv_MSS(){
    std::string conjunction_rules = "";

    // ---- for candidate set: ----
    // true:
    conjunction_rules += TRUTH_VALUE_PREDICATE_CS + "(X," + TRUTH_VALUE_T + "):-" + CONJUNCTION + "(X),N{" + TRUTH_VALUE_PREDICATE_CS + "(Z," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(Z,X)}N," + NUM_CONJUNCTS + "(X,N).";
    // false:
    conjunction_rules += TRUTH_VALUE_PREDICATE_CS + "(X," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X),1{" + TRUTH_VALUE_PREDICATE_CS + "(Z," + TRUTH_VALUE_F + "):" + CONJUNCT_OF + "(Z,X)}.";

    // ---- for supersets: ----
    conjunction_rules += TRUTH_VALUE_PREDICATE_SET + "(X,Y," + TRUTH_VALUE_T + "):-" + CONJUNCTION + "(X)," + IS_SUPERSET + "(Y),N{" + TRUTH_VALUE_PREDICATE_SET + "(Z,Y," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(Z,X)}N," + NUM_CONJUNCTS + "(X,N).";
    conjunction_rules += TRUTH_VALUE_PREDICATE_SET + "(X,Y," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X)," + IS_SUPERSET + "(Y),1{" + TRUTH_VALUE_PREDICATE_SET + "(Z,Y," + TRUTH_VALUE_F + "):" + CONJUNCT_OF + "(Z,X)}.";

    return conjunction_rules;
}

std::string add_disjunction_rules_mv_MSS(){
    std::string disjunction_rules = "";

    // ---- for candidate set: ----
    // false:
    disjunction_rules += TRUTH_VALUE_PREDICATE_CS + "(X," + TRUTH_VALUE_F + "):-" + DISJUNCTION + "(X),N{" + TRUTH_VALUE_PREDICATE_CS + "(Z," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(Z,X)}N," + NUM_DISJUNCTS + "(X,N).";
    // true:
    // disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + ").";
    disjunction_rules += TRUTH_VALUE_PREDICATE_CS + "(X," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X),1{" + TRUTH_VALUE_PREDICATE_CS + "(Z," + TRUTH_VALUE_T + "):" + DISJUNCT_OF + "(Z,X)}.";

    // ---- for supersets: ----
    disjunction_rules += TRUTH_VALUE_PREDICATE_SET + "(X,Y," + TRUTH_VALUE_F + "):-" + DISJUNCTION + "(X)," + IS_SUPERSET + "(Y),N{" + TRUTH_VALUE_PREDICATE_SET + "(Z,Y," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(Z,X)}N," + NUM_DISJUNCTS + "(X,N).";
    disjunction_rules += TRUTH_VALUE_PREDICATE_SET + "(X,Y," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X)," + IS_SUPERSET + "(Y),1{" + TRUTH_VALUE_PREDICATE_SET + "(Z,Y," + TRUTH_VALUE_T + "):" + DISJUNCT_OF + "(Z,X)}.";

    return disjunction_rules;
}

std::string add_negation_rules_mv_MSS(){
    std::string negation_rules = "";

    // ---- for candidate set: ----
    // true:
    negation_rules += TRUTH_VALUE_PREDICATE_CS + "(X," + TRUTH_VALUE_T + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_CS + "(Z," + TRUTH_VALUE_F + ").";
    // false:
    negation_rules += TRUTH_VALUE_PREDICATE_CS + "(X," + TRUTH_VALUE_F + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_CS + "(Z," + TRUTH_VALUE_T + ").";

    // ---- for supersets: ----

    negation_rules += TRUTH_VALUE_PREDICATE_SET + "(X,Y," + TRUTH_VALUE_T + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_SET + "(Z,Y," + TRUTH_VALUE_F + ").";
    negation_rules += TRUTH_VALUE_PREDICATE_SET + "(X,Y," + TRUTH_VALUE_F + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_SET + "(Z,Y," + TRUTH_VALUE_T + ").";

    return negation_rules;
}

// for optimization version:
std::string add_conjunction_rules_mv_opt(){
    std::string conjunction_rules = "";

    // true:
    conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,C," + TRUTH_VALUE_T + "):-" + CONJUNCTION + "(X)," + INTERPRETATION_OF + "(Y,C),N{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y,C," + TRUTH_VALUE_T + "):" + CONJUNCT_OF + "(Z,X)}N," + NUM_CONJUNCTS + "(X,N).";

    // false:
    // conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + ").";
    conjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,C," + TRUTH_VALUE_F + "):-" + CONJUNCTION + "(X)," + INTERPRETATION_OF + "(Y,C),1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y,C," + TRUTH_VALUE_F + "):" + CONJUNCT_OF + "(Z,X)}."; // N," + NUM_CONJUNCTS + "(X,N).";

    return conjunction_rules;
}

std::string add_disjunction_rules_mv_opt(){
    std::string disjunction_rules = "";

    // false:
    disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,C," + TRUTH_VALUE_F + "):-" + DISJUNCTION + "(X)," + INTERPRETATION_OF + "(Y,C),N{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y,C," + TRUTH_VALUE_F + "):" + DISJUNCT_OF + "(Z,X)}N," + NUM_DISJUNCTS + "(X,N).";
    // true:
    // disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X)," + INTERPRETATION +"(Y),not " + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y," + TRUTH_VALUE_F + ").";
    disjunction_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,C," + TRUTH_VALUE_T + "):-" + DISJUNCTION + "(X)," + INTERPRETATION_OF + "(Y,C),1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y,C," + TRUTH_VALUE_T + "):" + DISJUNCT_OF + "(Z,X)}."; // N," + NUM_DISJUNCTS + "(X,N).";

    return disjunction_rules;
}

std::string add_negation_rules_mv_opt(){
    std::string negation_rules = "";

    // true:
    negation_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,C," + TRUTH_VALUE_T + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y,C," + TRUTH_VALUE_F + ").";
    // false:
    negation_rules += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,C," + TRUTH_VALUE_F + "):-" + NEGATION + "(Z,X)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(Z,Y,C," + TRUTH_VALUE_T + ").";

    return negation_rules;
}

std::string add_general_rules_iterative(){
    std::string program = "";

    // add truth value facts:
    program += "tv(" + TRUTH_VALUE_T + ";" + TRUTH_VALUE_F + ").";

    // add predicates for retrieving atoms and KB elements:
    // program += "atom(A) :- atomInFormula(A,_).";
    program += ATOM + "(A):-" + ATOM_IN_FORMULA_MV + "(A,_).";
    // program += "kbElement(F) :- atomInFormula(_,F).";
    program += KB_MEMBER + "(F):-" + ATOM_IN_FORMULA_MV + "(_,F).";

    // if a formula is in fact an atom:
    program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,Z):tv(Z):-" + FORMULA_IS_ATOM + "(X,W)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(W,Y,Z)," + INTERPRETATION + "(Y).";


    return program;
}

std::string add_general_rules_iterative_MSS(){
    std::string program = "";

    // add truth value facts:
    program += "tv(" + TRUTH_VALUE_T + ";" + TRUTH_VALUE_F + ").";

    // add predicates for retrieving atoms and KB elements:
    // program += "atom(A) :- atomInFormula(A,_).";
    program += ATOM + "(A):-" + ATOM_IN_FORMULA_MV + "(A,_).";
    // program += "kbElement(F) :- atomInFormula(_,F).";
    program += KB_MEMBER + "(F):-" + ATOM_IN_FORMULA_MV + "(_,F).";

    // if a formula is in fact an atom:
    // truthValueCS(F,T) :- tv(T), formulaIsAtom(F,A), truthValueCS(A,T).
    program += TRUTH_VALUE_PREDICATE_CS + "(F,T):-tv(T)," + FORMULA_IS_ATOM + "(F,A)," + TRUTH_VALUE_PREDICATE_CS + "(A,T).";
    // truthValueSet(F,S,T) :- tv(T), formulaIsAtom(F,A), truthValueSet(A,S,T), isSuperset(S).
    program += TRUTH_VALUE_PREDICATE_SET + "(X,Y,Z):tv(Z):-" + FORMULA_IS_ATOM + "(X,W)," + TRUTH_VALUE_PREDICATE_SET + "(W,Y,Z)," + IS_SUPERSET + "(Y).";

    return program;
}

std::string add_cs_rules_iterative(){
    std::string program = "";

    // create candidate set:
    program += "1{" + IN_CS + "(X):" + KB_MEMBER + "(X)}."; // "N:-" + NUM_KB_ELEMENTS + "(N).";

    // ensure that query atom is contained in the cs:
    // atomInCs(A) :- atomInFormula(A,F), inCs(F).
    // :- not atomInCs(A), queryAtom(A).
    program += ATOM_IN_CS + "(A):-" + ATOM_IN_FORMULA_MV + "(A,F)," + IN_CS + "(F).";
    program += ":-not " + ATOM_IN_CS + "(A)," + QUERY_ATOM + "(A).";

    // count number of elements in cs:
    // numElementsInCs(X) :- X = #count{F: inCs(F)}.
    program += NUM_ELEMENTS_IN_CS + "(X):-X=#count{F:" + IN_CS + "(F)}.";

    // number of elements per subset:
    // numElementsInSubset(X) :- numElementsInCs(Y), X = Y-1.
    program += NUM_ELEMENTS_IN_SUBSET + "(X):-" + NUM_ELEMENTS_IN_CS + "(Y),X=Y-1.";

    return program;
}

std::string add_cs_rules_iterative_MSS(){
    std::string program = "";

    // create candidate set:
    program += "{" + IN_CS + "(X):" + KB_MEMBER + "(X)}."; // "N:-" + NUM_KB_ELEMENTS + "(N).";

    // Auxiliary predicate that tells whether a formula is NOT in the CS:
    // not_inCs(F) :- kbMember(F), not inCs(F).
    program += NOT_IN_CS + "(F):-" + KB_MEMBER + "(F),not " + IN_CS + "(F).";

    // Check if an atom is included in a formula that is not in the CS:
    // atomInRemainder(A) :- atomInFormula(A,F), not_inCs(F).
    program += ATOM_IN_REMAINDER + "(A):-" + ATOM_IN_FORMULA_MV + "(A,F)," + NOT_IN_CS + "(F).";

    // The query atom must be included in at least one formula outside the CS:
    // :- not atomInRemainder(A), queryAtom(A).
    program += ":-not " + ATOM_IN_REMAINDER + "(A)," + QUERY_ATOM + "(A).";

    // atomInCs(A) :- atomInFormula(A,F), inCs(F).
    program += ATOM_IN_CS + "(A):-" + ATOM_IN_FORMULA_MV + "(A,F)," + IN_CS + "(F).";

    // count number of elements in cs:
    // numElementsInCs(X) :- X = #count{F: inCs(F)}.
    program += NUM_ELEMENTS_IN_CS + "(X):-X=#count{F:" + IN_CS + "(F)}.";


    return program;
}

std::string add_cs_rules_iterative_MSS_2(){
    std::string program = "";

    // create candidate set:
    program += "{" + IN_CS + "(X):" + KB_MEMBER + "(X)}."; // "N:-" + NUM_KB_ELEMENTS + "(N).";

    // Auxiliary predicate that tells whether a formula is NOT in the CS:
    // not_inCs(F) :- kbMember(F), not inCs(F).
    program += NOT_IN_CS + "(F):-" + KB_MEMBER + "(F),not " + IN_CS + "(F).";

    // Check if an atom is included in a formula that is not in the CS:
    // atomInRemainder(A) :- atomInFormula(A,F), not_inCs(F).
    program += ATOM_IN_REMAINDER + "(A):-" + ATOM_IN_FORMULA_MV + "(A,F)," + NOT_IN_CS + "(F).";

    // The query atom must be included in at least one formula outside the CS:
    // :- not atomInRemainder(A), queryAtom(A).
    // program += ":-not " + ATOM_IN_REMAINDER + "(A)," + QUERY_ATOM + "(A).";
    program += "validCS:-1{" + ATOM_IN_REMAINDER + "(A):" + QUERY_ATOM + "(A)}.";
    program += ":-not validCS.";

    // atomInCs(A) :- atomInFormula(A,F), inCs(F).
    program += ATOM_IN_CS + "(A):-" + ATOM_IN_FORMULA_MV + "(A,F)," + IN_CS + "(F).";

    // count number of elements in cs:
    // numElementsInCs(X) :- X = #count{F: inCs(F)}.
    program += NUM_ELEMENTS_IN_CS + "(X):-X=#count{F:" + IN_CS + "(F)}.";


    return program;
}

std::string add_cs_sat_rules_iterative(){
    std::string program = "";

    // Assign each atom exactly one truth value wrt. each interpretation:
    // 1{truthValueCS(A,T): tv(T)}1 :- atomInCs(A).
    program += "1{" + TRUTH_VALUE_PREDICATE_CS + "(A,T):tv(T)}1:-" + ATOM_IN_CS + "(A).";

    // The CS is satisfiable if all of its formulas can be evaluated to true:
    // csIsSat :- numElementsInCs(X), X{truthValueCS(F,t): inCs(F), kbMember(F)}X.
    program += CS_IS_SAT + ":-" + NUM_ELEMENTS_IN_CS + "(X),X{" + TRUTH_VALUE_PREDICATE_CS + "(F," + TRUTH_VALUE_T + "):" + IN_CS + "(F)," + KB_MEMBER + "(F)}X.";
    // :- not csIsSat.
    program += ":-not " + CS_IS_SAT + ".";

    return program;
}

std::string add_cs_superset_rules_iterative(){
    std::string program = "";

    // Number supersets required (i.e., |KB|-|CS|):
    // numSupersets(X) :- numElementsInCs(Y), numKbElements(Z), X = Z-Y.
    program += NUM_SUPERSETS + "(X):-" + NUM_ELEMENTS_IN_CS + "(Y)," + NUM_KB_ELEMENTS + "(Z),X=Z-Y.";

    // Define supersets:
    // isSuperset(1..X) :- numSupersets(X), X>0.
    program += IS_SUPERSET + "(1..X):-" + NUM_SUPERSETS + "(X),X>0.";

    // Every formula that is not in the CS must be added to the CS once to form a superset:
    // 1{additionalElement(F,S): not_inCs(F)}1 :- isSuperset(S).
    program += "1{" + ADDITIONAL_ELEMENT + "(F,S):" + NOT_IN_CS + "(F)}1:-" + IS_SUPERSET + "(S).";

    // % No two supersets are allowed to be the same:
    // supersetEq(S1,S2) :- isSuperset(S1), isSuperset(S2), S1!=S2, additionalElement(F1,S1), additionalElement(F2,S2), F1==F2.
    // :- supersetEq(S1,S2).
    program += SUPERSET_EQ + "(S1,S2):-" + IS_SUPERSET + "(S1)," + IS_SUPERSET + "(S2),S1!=S2," + ADDITIONAL_ELEMENT + "(F1,S1)," + ADDITIONAL_ELEMENT + "(F2,S2),F1==F2.";
    program += ":-" + SUPERSET_EQ + "(S1,S2).";

    // Define which formulas are included in each superset:
    // inSuperset(F,S) :- inCs(F), isSuperset(S).
    // inSuperset(F,S) :- additionalElement(F,S), isSuperset(S).
    program += IN_SUPERSET + "(F,S):-" + IN_CS + "(F)," + IS_SUBSET + "(S).";
    program += IN_SUPERSET + "(F,S):-" + ADDITIONAL_ELEMENT + "(F,S)," + IS_SUPERSET + "(S).";

    // atomInSuperset(A,S) :- atomInCs(A), isSuperset(S).
    // atomInSuperset(A,S) :- additionalElement(F,S), atomInFormula(A,F).
    program += ATOM_IN_SUPERSET + "(A,S):-" + ATOM_IN_CS + "(A)," + IS_SUPERSET + "(S).";
    program += ATOM_IN_SUPERSET + "(A,S):-" + ADDITIONAL_ELEMENT + "(F,S)," + ATOM_IN_FORMULA_MV + "(A,F).";

    return program;
}

std::string add_subset_sat_rules_iterative(){
    std::string program = "";

    // define subsets:
    // isSubset(1..X) :- numElementsInCs(X).
    // Y{inSubset(F,S): inCs(F)}Y :- isSubset(S), numElementsInSubset(Y).
    program += IS_SUBSET + "(1..X):-" + NUM_ELEMENTS_IN_CS + "(X),X>1.";
    program += "Y{" + IN_SUBSET + "(F,S):" + IN_CS + "(F)}Y:-" + IS_SUBSET + "(S)," + NUM_ELEMENTS_IN_SUBSET + "(Y).";

    // ensure that all subsets are different:
    // 	-> x = number of formulas that are the same
    // subsetEq(S1,S2) :- isSubset(S1), isSubset(S2), S1 != S2, X = #count{F: inSubset(F,S1), inSubset(F,S2)}, numElementsInSubset(Y), X == Y.
    // :- subsetEq(S1,S2). %, isSubset(S1), isSubset(S2).
    program += SUBSET_EQ + "(S1,S2):-" + IS_SUBSET + "(S1)," + IS_SUBSET + "(S2),S1!=S2,X=#count{F:" + IN_SUBSET + "(F,S1)," + IN_SUBSET + "(F,S2)}," + NUM_ELEMENTS_IN_SUBSET + "(Y),X==Y.";
    program += ":-" + SUBSET_EQ + "(S1,S2).";

    // define |cs| interpretations:
    // interpretation(1..X) :- numElementsInCs(X).
    program += INTERPRETATION + "(1..X):-" + NUM_ELEMENTS_IN_CS + "(X),X>1.";
    // guess atom evaluations:
    // 1{truthValueInt(A,I,T) : tv(T)}1 :- atom(A), interpretation(I), I != u.
    program += "1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,I,T):tv(T)}1:-" + ATOM_IN_CS + "(A)," + INTERPRETATION + "(I),I!=u.";

    // subset must be satisfiable under at least one interpretation
    // subsetIsSat(S) :- isSubset(S), numElementsInSubset(X), interpretation(I), X{truthValueInt(F,I,t): inSubset(F,S), kbElement(F)}X, I != u.
    program += SUBSET_IS_SAT + "(S):-" + IS_SUBSET + "(S)," + NUM_ELEMENTS_IN_SUBSET + "(X)," + INTERPRETATION + "(I),X{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,I," + TRUTH_VALUE_T + "):" + IN_SUBSET + "(F,S)," + KB_MEMBER + "(F)}X,I!=u.";

    // ensure that all subsets are satisfiable:
    // :- not subsetIsSat(S), isSubset(S).
    program += ":-not " + SUBSET_IS_SAT + "(S)," + IS_SUBSET + "(S).";

    return program;
}

std::string add_saturation_iterative(){
    std::string program = "";

    // ---- Unsatisfiablity check part (-> saturation part) ----
    program += INTERPRETATION + "(u).";

    // truthValueInt(A,u,t) | truthValueInt(A,u,f) :- atom(A).
    program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u," + TRUTH_VALUE_T  + ")|" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u," + TRUTH_VALUE_F + "):-" + ATOM_IN_CS + "(A).";

    // isUnsat :- truthValueInt(F,u,f), inCs(F).
    program += IS_UNSAT + ":-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,u," + TRUTH_VALUE_F + ")," + IN_CS + "(F)," + KB_MEMBER + "(F).";
    // program += "isUnsat :- truthValueInt(F,u,tv_f), inCs(F).";

    // :- not isUnsat.
    program += ":-not " + IS_UNSAT + ".";

    // truthValueInt(A,u,t) :- isUnsat, atom(A).
    // truthValueInt(A,u,f) :- isUnsat, atom(A). // , numElementsInSubset(X), X>0.
    program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u," + TRUTH_VALUE_T + "):-" + IS_UNSAT + "," + ATOM_IN_CS + "(A).";
    // program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u," + TRUTH_VALUE_F + "):-" + IS_UNSAT + "," + ATOM + "(A)," + NUM_ELEMENTS_IN_CS + "(X),X>1.";
    program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u," + TRUTH_VALUE_F + "):-" + IS_UNSAT + "," + ATOM_IN_CS + "(A).";

    return program;
}

std::string add_saturation_iterative_MSS(){
    std::string program = "";

    // ---- Unsatisfiablity check part (-> saturation part) ----

    // truthValueSet(A,S,t) | truthValueSet(A,S,f) :- atomInSuperset(A,S), isSuperset(S).
    program += TRUTH_VALUE_PREDICATE_SET + "(A,S," + TRUTH_VALUE_T  + ")|" + TRUTH_VALUE_PREDICATE_SET + "(A,S," + TRUTH_VALUE_F + "):-" + ATOM_IN_SUPERSET + "(A,S)," + IS_SUPERSET + "(S).";

    // isUnsat(S) :- truthValueSet(F,S,f), inSuperset(F,S).
    program += IS_UNSAT + "(S):-" + TRUTH_VALUE_PREDICATE_SET + "(F,S," + TRUTH_VALUE_F + ")," + IN_SUPERSET + "(F,S).";

    // :- not isUnsat(S), isSuperset(S).
    program += ":-not " + IS_UNSAT + "(S)," + IS_SUPERSET + "(S).";


    // truthValueSet(A,S,t) :- isUnsat(S), atomInSuperset(A,S), isSuperset(S).
    // truthValueSet(A,S,f) :- isUnsat(S), atomInSuperset(A,S), isSuperset(S).
    program += TRUTH_VALUE_PREDICATE_SET + "(A,S," + TRUTH_VALUE_T + "):-" + IS_UNSAT + "(S)," + ATOM_IN_SUPERSET + "(A,S)," + IS_SUPERSET + "(S).";
    program += TRUTH_VALUE_PREDICATE_SET + "(A,S," + TRUTH_VALUE_F + "):-" + IS_UNSAT + "(S)," + ATOM_IN_SUPERSET + "(A,S)," + IS_SUPERSET + "(S).";

    return program;
}

// mv
double mv_measure_iterative(Kb& kb){

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

        int num_atoms_in_MIS = 0;

        // iterate over signature
        // for each atom, check if there is an answer set
        std::set<std::string> signature = kb.GetSignature();

        // initialize program string:
        std::string program = "";

        // ---- set up program without query atom ---

        // add number of KB elements:
        // program += NUM_KB_ELEMENTS + "(" + std::to_string(numFormulas) + ").";

        // add rules for each formula:
        program += handle_formulas_in_kb_mv(kb);

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_mv();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_mv();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_mv();

        program += add_general_rules_iterative();
        program += add_cs_rules_iterative();
        program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative();

        // std::cout << program << std::endl;

        for(std::string q_atom : signature){

            // add query atom:
            std::transform(q_atom.begin(), q_atom.end(), q_atom.begin(),
                [](unsigned char c){ return std::tolower(c); });
            std::string query_atom = QUERY_ATOM + "(" + q_atom + ").";

            // add query atom to program:
            std::string curr_program = program + query_atom;

            // ------------------------------------------------------------------

            auto encoding_end = std::chrono::steady_clock::now();
            double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                              / double (1000000.0);
            time_msrs::encoding_times.push_back(encoding_interval);

            time_msrs::num_solver_calls++;
            // Measure the time clingo takes to solve the program
            auto solver_start = std::chrono::steady_clock::now();

            // let Clingo solve the problem; check if an answer set exists:
            // int opt = compute_optimum_with_inf(program);
            if(answerSetExists(curr_program)){
                num_atoms_in_MIS++;
            }

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

        }

        // std::cout << num_atoms_in_MIS << std::endl;

        return (double)num_atoms_in_MIS / (double)signature.size();
    }
}

// mv2
double mv_measure_optimize(Kb& kb){

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

        int num_atoms_in_MIS = 0;

        // iterate over signature
        // for each atom, check if there is an answer set
        std::set<std::string> signature = kb.GetSignature();

        // initialize program string:
        std::string program = "";

        // ---- set up program without query atom ---
        // add truth value facts:
        program += "tv(" + TRUTH_VALUE_T + ";" + TRUTH_VALUE_F + ").";

        // add predicates for retrieving atoms and KB elements:
        // program += "atom(A) :- atomInFormula(A,_).";
        program += ATOM + "(A):-" + ATOM_IN_FORMULA_MV + "(A,_).";
        // program += "kbElement(F) :- atomInFormula(_,F).";
        program += KB_MEMBER + "(F):-" + ATOM_IN_FORMULA_MV + "(_,F).";

        // add number of KB elements:
        program += NUM_KB_ELEMENTS + "(" + std::to_string(numFormulas) + ").";

        // add one candidate set per atom in the signature:
        int sig_count = 0;
        for (std::string a : signature){
            // TODO: make atom names lowercase
            std::string a_lc = a;
            std::transform(a_lc.begin(), a_lc.end(), a_lc.begin(),
                [](unsigned char c){ return std::tolower(c); });
            program += QUERY_ATOM_OF_SET + "(" + a_lc + ",cs" + std::to_string(sig_count) + ").";
            sig_count++;
        }

        // extract candidate sets:
        // isCs(C) :- queryAtomOfSet(_,C).
        program += IS_CS + "(C):-" + QUERY_ATOM_OF_SET + "(_,C).";

        // add rules for each formula:
        program += handle_formulas_in_kb_mv(kb);

        // ------------------------------------------------------

        // if a formula is in fact an atom:
        program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(X,Y,C,Z):-tv(Z)," + FORMULA_IS_ATOM + "(X,W)," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(W,Y,C,Z)," + INTERPRETATION_OF + "(Y,C).";

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_mv_opt();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_mv_opt();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_mv_opt();

        // create candidate sets:
        // 1{inCs(F,C): kbElement(F)} :- isCs(C).
        program += "1{" + IN_CS + "(F,C):" + KB_MEMBER + "(F)}:-" + IS_CS + "(C).";

        // % ensure that query atom is contained in the cs:
        // atomInCs(A,C) :- atomInFormula(A,F), inCs(F,C).
        // :- not atomInCs(A,C), queryAtomOfSet(A,C).
        program += ATOM_IN_CS + "(A,C):-" + ATOM_IN_FORMULA_MV + "(A,F)," + IN_CS + "(F,C).";
        program += ":-not " + ATOM_IN_CS + "(A,C)," + QUERY_ATOM_OF_SET + "(A,C).";

        // count number of elements in cs:
        // numElementsInCs(C,X) :- X = #count{F: inCs(F,C)}, isCs(C).
        program += NUM_ELEMENTS_IN_CS + "(C,X):-X=#count{F:" + IN_CS + "(F,C)}," + IS_CS + "(C).";

        // number of elements per subset:
        // numElementsInSubset(S,C,X) :- numElementsInCs(C,Y), subsetOfSet(S,C), X = Y-1.
        program += NUM_ELEMENTS_IN_SUBSET + "(S,C,X):-" + NUM_ELEMENTS_IN_CS + "(C,Y)," + SUBSET_OF_SET + "(S,C),X=Y-1.";

        // define subsets:
        // subsetOfSet(1..X,C) :- isCs(C), numElementsInCs(C,X), X>1.
        // Y{inSubsetOfSet(F,S,C): inCs(F,C)}Y :- subsetOfSet(S,C), numElementsInSubset(S,C,Y).
        program += SUBSET_OF_SET + "(1..X,C):-" + IS_CS + "(C)," + NUM_ELEMENTS_IN_CS + "(C,X),X>1.";
        program += "Y{" + IN_SUBSET_OF_SET + "(F,S,C):" + IN_CS + "(F,C)}Y:-" + SUBSET_OF_SET + "(S,C)," + NUM_ELEMENTS_IN_SUBSET + "(S,C,Y).";

        // ensure that all subsets are different wrt. a cs:
        // subsetEq(S1,S2,C) :- subsetOfSet(S1,C), subsetOfSet(S2,C), S1 != S2, X = #count{F: inSubsetOfSet(F,S1,C), inSubsetOfSet(F,S2,C)},
        //      numElementsInSubset(S1,C,Y), X == Y.
        program += SUBSET_EQ + "(S1,S2,C):-" + SUBSET_OF_SET + "(S1,C)," + SUBSET_OF_SET + "(S2,C),S1!=S2,X=#count{F:" + IN_SUBSET_OF_SET + "(F,S1,C)," + IN_SUBSET_OF_SET + "(F,S2,C)}," + NUM_ELEMENTS_IN_SUBSET + "(S1,C,Y),X==Y.";
        program += ":-" + SUBSET_EQ + "(S1,S2,C).";

        // define |cs| interpretations per cs:
        // interpretationOf(1..X,C) :- isCs(C), numElementsInCs(C,X), X>1..
        program += INTERPRETATION_OF + "(1..X,C):-" + IS_CS + "(C)," + NUM_ELEMENTS_IN_CS + "(C,X),X>1.";

        // guess atom evaluations:
        // 1{truthValueInt(A,I,C,T) : tv(T)}1 :- atom(A), interpretationOf(I,C), I != u.
        program += "1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,I,C,T):tv(T)}1:-" + ATOM + "(A)," + INTERPRETATION_OF + "(I,C),I!=u.";

        // subset must be satisfiable under at least one interpretation
        // subsetIsSat(S,C) :- subsetOfSet(S,C), numElementsInSubset(S,C,X), interpretationOf(I,C), X{truthValueInt(F,I,C,t): inSubsetOfSet(F,S,C), kbElement(F)}X, I != u.
        program += SUBSET_IS_SAT + "(S,C):-" + SUBSET_OF_SET + "(S,C)," + NUM_ELEMENTS_IN_SUBSET + "(S,C,X)," + INTERPRETATION_OF + "(I,C),X{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,I,C," + TRUTH_VALUE_T + "):" + IN_SUBSET_OF_SET + "(F,S,C)," + KB_MEMBER + "(F)}X,I!=u.";

        // ensure that all subsets are satisfiable:
        // :- not subsetIsSat(S,C), subsetOfSet(S,C).
        program += ":-not " + SUBSET_IS_SAT + "(S,C)," + SUBSET_OF_SET + "(S,C).";

        // ---- Unsatisfiablity check part (-> saturation part) ----
        // interpretationOf(u,C) :- isCs(C).
        program += INTERPRETATION_OF + "(u,C):-" + IS_CS + "(C).";

        // truthValueInt(A,u,C,t) | truthValueInt(A,u,C,f) :- atomInCs(A,C).
        program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u,C," + TRUTH_VALUE_T  + ")|" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u,C," + TRUTH_VALUE_F + "):-" + ATOM_IN_CS + "(A,C).";

        // isUnsat(C) :- truthValueInt(F,u,C,f), inCs(F,C).
        program += IS_UNSAT + "(C):-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,u,C," + TRUTH_VALUE_F + ")," + IN_CS + "(F,C)."; // + KB_MEMBER + "(F).";

        // :- not isUnsat.
        // program += ":-not " + IS_UNSAT + ".";

        // truthValueInt(A,u,C,t) :- isUnsat(C), atomInCs(A,C).
        // truthValueInt(A,u,C,f) :- isUnsat(C), atomInCs(A,C).
        program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u,C," + TRUTH_VALUE_T + "):-" + IS_UNSAT + "(C)," + ATOM_IN_CS + "(A,C).";
        // program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u," + TRUTH_VALUE_F + "):-" + IS_UNSAT + "," + ATOM + "(A)," + NUM_ELEMENTS_IN_CS + "(X),X>1.";
        program += TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u,C," + TRUTH_VALUE_F + "):-" + IS_UNSAT + "(C)," + ATOM_IN_CS + "(A,C).";

        // number of atoms in a cs:
        // numAtomsInCs(C,X) :- X = #count{A: atomInCs(A,C)}, isCs(C).
        program += NUM_ATOMS_IN_CS + "(C,X):-X=#count{A:" + ATOM_IN_CS + "(A,C)}," + IS_CS + "(C).";

        // CS is unsat if all atoms are both t and f (under interpretation u):
        // bothTrueAndFalse(A,C) :- truthValueInt(A,u,C,t), truthValueInt(A,u,C,f), atomInCs(A,C).
        // csIsUnsat(C) :- isCs(C), X{bothTrueAndFalse(A,C): atomInCs(A,C)}X, numAtomsInCs(C,X).
        program += BOTH_T_AND_F + "(A,C):-" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u,C," + TRUTH_VALUE_T + ")," + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,u,C," + TRUTH_VALUE_F + ")," + ATOM_IN_CS + "(A,C).";
        program += CS_IS_UNSAT + "(C):-" + IS_CS + "(C),X{" + BOTH_T_AND_F + "(A,C):" + ATOM_IN_CS + "(A,C)}X," + NUM_ATOMS_IN_CS + "(C,X).";

        // check if a cs is an MIS (2 cases):
        // csIsMIS(C) :- isCs(C), csIsUnsat(C), numElementsInCs(C,X), X==1.
        // csIsMIS(C) :- isCs(C), csIsUnsat(C), X{subsetIsSat(S,C): subsetOfSet(S,C)}X, numElementsInCs(C,X), X>1.
        program += CS_IS_MIS + "(C):-" + IS_CS + "(C)," + CS_IS_UNSAT + "(C)," + NUM_ELEMENTS_IN_CS + "(C,X),X==1.";
        program += CS_IS_MIS + "(C):-" + IS_CS + "(C)," + CS_IS_UNSAT + "(C),X{" + SUBSET_IS_SAT + "(S,C):" + SUBSET_OF_SET + "(S,C)}X," + NUM_ELEMENTS_IN_CS + "(C,X),X>1.";

        // check if an atom is in an MIS:
        // atomInMIS(A) :- csIsMIS(C), atomInCs(A,C).
        program += ATOM_IN_MIS + "(A):-" + CS_IS_MIS + "(C)," + ATOM_IN_CS + "(A,C).";

        // maximization:
        // #maximize{1,A: atomInMIS(A)}.
        program += "#maximize{1,A:" + ATOM_IN_MIS + "(A)}.";

        // std::cout << program << std::endl;

        // ------------------------------------------------------------------
        // solving:

        auto encoding_end = std::chrono::steady_clock::now();
        double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                        / double (1000000.0);
        time_msrs::encoding_times.push_back(encoding_interval);

        time_msrs::num_solver_calls++;
        // Measure the time clingo takes to solve the program
        auto solver_start = std::chrono::steady_clock::now();

        // let Clingo solve the problem; retrieve optimum
        int opt = compute_optimum(program) * (-1);

        auto solver_end = std::chrono::steady_clock::now();
        double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                        / double (1000000.0);
        time_msrs::solver_times_total.push_back(solver_interval);

        // std::cout << num_atoms_in_MIS << std::endl;

        // return (double)num_atoms_in_MIS / (double)signature.size();
        return (double)opt / (double)signature.size();
    }
}

// mv3b
double mv_measure_iterative_2(Kb& kb){

    // In this version, the satisfiability check part is different from the original

    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        // std::cout << "Inconsistency value: 0" << std::endl;
        return 0;
    }

    else{

        int num_atoms_in_MIS = 0;

        // iterate over signature
        // for each atom, check if there is an answer set
        std::set<std::string> signature = kb.GetSignature();

        // initialize program string:
        std::string program = "";

        // add number of KB elements:
        program += NUM_KB_ELEMENTS + "(" + std::to_string(numFormulas) + ").";

        // add rules for each formula:
        program += handle_formulas_in_kb_mv(kb);

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_mv();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_mv();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_mv();

        program += add_general_rules_iterative();
        program += add_cs_rules_iterative();
        // program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative();

        // define subsets:
        // isSubset(1..X) :- numElementsInCs(X).
        program += IS_SUBSET + "(1..X):-" + NUM_ELEMENTS_IN_CS + "(X),X>1.";
        // 1{remove(F,S): inCs(F)}1 :- isSubset(S).
        program += "1{" + REMOVE_F + "(F,S):" + IN_CS + "(F)}1:-" + IS_SUBSET + "(S).";

        // ensure that all subsets are different:
        // 	-> x = number of formulas that are the same
        // removeEq(S1,S2) :- isSubset(S1), isSubset(S2), S1 != S2, remove(F1,S1), remove(F2,S2), F1 == F2.
        // :- removeEq(S1,S2).
        program += REMOVE_EQ + "(S1,S2):-" + IS_SUBSET + "(S1)," + IS_SUBSET + "(S2),S1!=S2," + REMOVE_F + "(F1,S1)," + REMOVE_F + "(F2,S2),F1==F2." ;
        program += ":-" + REMOVE_EQ + "(S1,S2).";

        // define |cs| interpretations:
        // interpretation(1..X) :- numElementsInCs(X).
        program += INTERPRETATION + "(1..X):-" + NUM_ELEMENTS_IN_CS + "(X),X>1.";

        // guess atom evaluations:
        // 1{truthValueInt(A,I,T): tv(T)}1 :- atomInCs(A), interpretation(I), I != u.
        program += "1{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(A,I,T):tv(T)}1:-" + ATOM_IN_CS + "(A)," + INTERPRETATION + "(I),I!=u.";

        // subset must be satisfiable under at least one interpretation
        // subsetIsSat(S) :- isSubset(S), remove(R,S), numElementsInSubset(X), interpretation(I), X{truthValueInt(F,I,t): inCs(F), F != R}X, I != u.
        program += SUBSET_IS_SAT + "(S):-" + IS_SUBSET + "(S)," + REMOVE_F + "(R,S)," + NUM_ELEMENTS_IN_SUBSET + "(X)," + INTERPRETATION + "(I),X{" + TRUTH_VALUE_PREDICATE_INTERPRETATION + "(F,I," + TRUTH_VALUE_T + "):" + IN_CS + "(F),F!=R}X,I!=u.";

        // ensure that all subsets are satisfiable:
        // :- not subsetIsSat(S), isSubset(S).
        program += ":-not " + SUBSET_IS_SAT + "(S)," + IS_SUBSET + "(S).";

        for(std::string q_atom : signature){

            // add query atom:
            std::transform(q_atom.begin(), q_atom.end(), q_atom.begin(),
                [](unsigned char c){ return std::tolower(c); });
            std::string query_atom = QUERY_ATOM + "(" + q_atom + ").";

            // add query atom to program:
            std::string curr_program = program + query_atom;

            // ------------------------------------------------------------------

            auto encoding_end = std::chrono::steady_clock::now();
            double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                              / double (1000000.0);
            time_msrs::encoding_times.push_back(encoding_interval);

            time_msrs::num_solver_calls++;
            // Measure the time clingo takes to solve the program
            auto solver_start = std::chrono::steady_clock::now();

            // let Clingo solve the problem; check if an answer set exists:
            // int opt = compute_optimum_with_inf(program);
            if(answerSetExists(curr_program)){
                num_atoms_in_MIS++;
            }

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

        }

        // std::cout << num_atoms_in_MIS << std::endl;

        return (double)num_atoms_in_MIS / (double)signature.size();
    }
}

// mv3
double mv_measure_iterative_3(Kb& kb){

    // In this version, the procedure uses potentially fewer iterations by checking which (other) atoms are in the MUSes

    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        // std::cout << "Inconsistency value: 0" << std::endl;
        return 0;
    }

    else{

        // int num_atoms_in_MIS = 0;

        // iterate over signature
        // for each atom, check if there is an answer set
        std::set<std::string> signature = kb.GetSignature();

        // initialize program string:
        std::string program = "";

        // ---- set up program without query atom ---

        // add rules for each formula:
        program += handle_formulas_in_kb_mv(kb);

        // add universal rules for connectors:
        if(program.find(CONJUNCTION) != std::string::npos)
            program += add_conjunction_rules_mv();
        if(program.find(DISJUNCTION) != std::string::npos)
            program += add_disjunction_rules_mv();
        if(program.find(NEGATION) != std::string::npos)
            program += add_negation_rules_mv();

        program += add_general_rules_iterative();
        program += add_cs_rules_iterative();
        program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative();

        // std::cout << program << std::endl;

        std::set<std::string> atoms_in_MUS; // = kb.GetSignature();

        for(std::string q_atom : signature){

            // add query atom:
            std::transform(q_atom.begin(), q_atom.end(), q_atom.begin(),
                [](unsigned char c){ return std::tolower(c); });

            bool is_in = atoms_in_MUS.find(q_atom) != atoms_in_MUS.end();
            if (is_in){
                continue;
            }

            std::string query_atom = QUERY_ATOM + "(" + q_atom + ").";

            // add query atom to program:
            std::string curr_program = program + query_atom;

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
            std::set<std::string> atoms_in_curr_MUS = getNumberOfAtomsInMUS(curr_program);
            for(auto a: atoms_in_curr_MUS){
                // std::cout << a << std::endl;
                atoms_in_MUS.insert(a);
            }

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

        }

        return (double)atoms_in_MUS.size() / (double)signature.size();
    }
}

// mv-mss
double mv_measure_iterative_MSS(Kb& kb){

    // This version checks whether an atom is included in an MCS (by checking whether it is contained in a formula in the complement of an MSS)
    // Additionally, the procedure uses potentially fewer iterations by checking which (other) atoms are in the MCSes

    // TODO: cover edge case in which every formula in the KB is contradictory

    int numFormulas = kb.size();
    // Measure time needed to build the ASP-Program from the given knowledge base
    auto encoding_start = std::chrono::steady_clock::now();
    // Check if KB is empty:
    if(numFormulas == 0){
        // std::cout << "Inconsistency value: 0" << std::endl;
        return 0;
    }

    else{

        // int num_atoms_in_MCS = 0;

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
        program += add_cs_rules_iterative_MSS();
        program += add_cs_sat_rules_iterative();
        program += add_cs_superset_rules_iterative();
        // program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative_MSS();

        // std::cout << program << std::endl;

        std::set<std::string> atoms_in_MCS; // = kb.GetSignature();

        for(std::string q_atom : signature){

            // add query atom:
            std::transform(q_atom.begin(), q_atom.end(), q_atom.begin(),
                [](unsigned char c){ return std::tolower(c); });

            bool is_in = atoms_in_MCS.find(q_atom) != atoms_in_MCS.end();
            if (is_in){
                continue;
            }

            std::string query_atom = QUERY_ATOM + "(" + q_atom + ").";

            // add query atom to program:
            std::string curr_program = program + query_atom;

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
            std::set<std::string> atoms_in_curr_MCS = getAtomsInMCS(curr_program);
            for(auto a: atoms_in_curr_MCS){
                // std::cout << a << std::endl;
                atoms_in_MCS.insert(a);
            }

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

        }

        return (double)atoms_in_MCS.size() / (double)signature.size();
    }
}

// mv-mss-2
double mv_measure_iterative_MSS_2(Kb& kb){

    // This version checks whether an atom is included in an MCS (by checking whether it is contained in a formula in the complement of an MSS)
    // Additionally, the procedure uses potentially fewer iterations by checking which (other) atoms are in the MCSes
    // This version also uses a more "flexible" iteration method by specifying that we are looking for an MCS containing *any* unseen atom.
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

        // int num_atoms_in_MCS = 0;

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
        program += add_cs_rules_iterative_MSS_2();
        program += add_cs_sat_rules_iterative();
        program += add_cs_superset_rules_iterative();
        // program += add_subset_sat_rules_iterative();
        program += add_saturation_iterative_MSS();

        // std::cout << program << std::endl;

        std::set<std::string> atoms_in_MCS; // = kb.GetSignature();

        std::set<std::string> remaining_atoms_tmp = kb.GetSignature();
        std::set<std::string> remaining_atoms;
        for(auto a: remaining_atoms_tmp){
            std::transform(a.begin(), a.end(), a.begin(),
                [](unsigned char c){ return std::tolower(c); });
            remaining_atoms.insert(a);
        }

        // for(auto a : remaining_atoms){
        //     std::cout << a << std::endl;
        // }

        while(!remaining_atoms.empty()){

            // add query atom:
            // std::transform(q_atom.begin(), q_atom.end(), q_atom.begin(),
            //     [](unsigned char c){ return std::tolower(c); });

            // bool is_in = atoms_in_MCS.find(q_atom) != atoms_in_MCS.end();
            // if (is_in){
            //     continue;
            // }

            // std::string query_atom = QUERY_ATOM + "(" + q_atom + ").";

            // add query atom to program:
            // std::string curr_program = program + query_atom;

            std::string all_query_atoms = "";

            for(auto a: remaining_atoms){
                std::string query_atom = QUERY_ATOM + "(" + a + ").";
                all_query_atoms += query_atom;
            }

            std::string curr_program = program + all_query_atoms;

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
            std::set<std::string> atoms_in_curr_MCS = getAtomsInMCS(curr_program);

            if (atoms_in_curr_MCS.empty()){
                break;
            }

            for(auto a: atoms_in_curr_MCS){
                // std::cout << a << std::endl;
                atoms_in_MCS.insert(a);
                remaining_atoms.erase(a);
            }

            auto solver_end = std::chrono::steady_clock::now();
            double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
                              / double (1000000.0);
            time_msrs::solver_times_total.push_back(solver_interval);

        }

        return (double)atoms_in_MCS.size() / (double)signature.size();
    }
}

// -----------------------------------------------------------------------------------------
// This does not work!
// double mv_measure_iterative_TEST(Kb& kb){

//     // In this version, the procedure uses potentially fewer iterations by checking which (other) atoms are in the MUSes

//     int numFormulas = kb.size();
//     // Measure time needed to build the ASP-Program from the given knowledge base
//     auto encoding_start = std::chrono::steady_clock::now();
//     // Check if KB is empty:
//     if(numFormulas == 0){
//         // std::cout << "Inconsistency value: 0" << std::endl;
//         return 0;
//     }

//     else{

//         // int num_atoms_in_MIS = 0;

//         // iterate over signature
//         // for each atom, check if there is an answer set
//         std::set<std::string> signature = kb.GetSignature();

//         // initialize program string:
//         std::string program = "";

//         // ---- set up program without query atom ---

//         std::string base_program = add_general_rules_iterative();
//         base_program += add_cs_rules_iterative();
//         base_program += add_subset_sat_rules_iterative();
//         base_program += add_saturation_iterative();

//         program += base_program;

//         // add rules for each formula:
//         program += handle_formulas_in_kb_mv(kb);

//         // add universal rules for connectors:
//         if(program.find(CONJUNCTION) != std::string::npos)
//             program += add_conjunction_rules_mv();
//         if(program.find(DISJUNCTION) != std::string::npos)
//             program += add_disjunction_rules_mv();
//         if(program.find(NEGATION) != std::string::npos)
//             program += add_negation_rules_mv();

//         // std::cout << program << std::endl;

//         std::set<std::string> atoms_in_MUS; // = kb.GetSignature();
//         bool new_atoms_in_MUS = false;

//         for(std::string q_atom : signature){

//             // add query atom:
//             std::transform(q_atom.begin(), q_atom.end(), q_atom.begin(),
//                 [](unsigned char c){ return std::tolower(c); });

//             bool is_in = atoms_in_MUS.find(q_atom) != atoms_in_MUS.end();
//             if (is_in){
//                 continue;
//             }

//             if (new_atoms_in_MUS == false){
//                 std::string query_atom = QUERY_ATOM + "(" + q_atom + ").";

//                 // add query atom to program:
//                 std::string curr_program = program + query_atom;

//                 // ------------------------------------------------------------------

//                 auto encoding_end = std::chrono::steady_clock::now();
//                 double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
//                                 / double (1000000.0);
//                 time_msrs::encoding_times.push_back(encoding_interval);

//                 time_msrs::num_solver_calls++;
//                 // Measure the time clingo takes to solve the program
//                 auto solver_start = std::chrono::steady_clock::now();

//                 // let Clingo solve the problem; check if an answer set exists:
//                 // also check which atoms are included in the MIS found
//                 std::set<std::string> atoms_in_curr_MUS = getNumberOfAtomsInMUS(curr_program);

//                 std::cout << "current MUS: ";
//                 for(auto a : atoms_in_curr_MUS){
//                     std::cout << a << ", ";
//                 }
//                 std::cout << std::endl;

//                 if (atoms_in_curr_MUS.size() > 0){
//                     new_atoms_in_MUS = true;
//                 }
//                 else{
//                     new_atoms_in_MUS = false;
//                 }

//                 for(auto a: atoms_in_curr_MUS){
//                     // std::cout << a << std::endl;
//                     atoms_in_MUS.insert(a);
//                 }

//                 std::cout << "overall MUS: ";
//                 for(auto a : atoms_in_MUS){
//                     std::cout << a << ", ";
//                 }
//                 std::cout << std::endl;

//                 auto solver_end = std::chrono::steady_clock::now();
//                 double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
//                                 / double (1000000.0);
//                 time_msrs::solver_times_total.push_back(solver_interval);
//             }
//             else{

//                 program = base_program;

//                 program += handle_formulas_in_kb_mv_new(kb, atoms_in_MUS);

//                 if(program.find(CONJUNCTION) != std::string::npos)
//                     program += add_conjunction_rules_mv();
//                 if(program.find(DISJUNCTION) != std::string::npos)
//                     program += add_disjunction_rules_mv();
//                 if(program.find(NEGATION) != std::string::npos)
//                     program += add_negation_rules_mv();

//                 std::string query_atom = QUERY_ATOM + "(" + q_atom + ").";

//                 // add query atom to program:
//                 std::string curr_program = program + query_atom;

//                 // ------------------------------------------------------------------

//                 auto encoding_end = std::chrono::steady_clock::now();
//                 double encoding_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
//                                 / double (1000000.0);
//                 time_msrs::encoding_times.push_back(encoding_interval);

//                 time_msrs::num_solver_calls++;
//                 // Measure the time clingo takes to solve the program
//                 auto solver_start = std::chrono::steady_clock::now();

//                 // let Clingo solve the problem; check if an answer set exists:
//                 // also check which atoms are included in the MIS found
//                 std::set<std::string> atoms_in_curr_MUS = getNumberOfAtomsInMUS(curr_program);

//                 std::cout << "current MUS: ";
//                 for(auto a : atoms_in_curr_MUS){
//                     std::cout << a << ", ";
//                 }
//                 std::cout << std::endl;

//                 if (atoms_in_curr_MUS.size() > 0){
//                     new_atoms_in_MUS = true;
//                 }
//                 else{
//                     new_atoms_in_MUS = false;
//                 }

//                 for(auto a: atoms_in_curr_MUS){
//                     // std::cout << a << std::endl;
//                     atoms_in_MUS.insert(a);
//                 }

//                 std::cout << "overall MUS: ";
//                 for(auto a : atoms_in_MUS){
//                     std::cout << a << ", ";
//                 }
//                 std::cout << std::endl;

//                 auto solver_end = std::chrono::steady_clock::now();
//                 double solver_interval = double (std::chrono::duration_cast<std::chrono::microseconds>(solver_end - solver_start).count())
//                                 / double (1000000.0);
//                 time_msrs::solver_times_total.push_back(solver_interval);

//             }

//         }

//         std::cout << "atoms_in_MUS size: " << atoms_in_MUS.size() << "\nKB signature size: " << signature.size() << std::endl;

//         return (double)atoms_in_MUS.size() / (double)signature.size();
//     }
// }
