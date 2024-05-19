#include <string>

#ifndef INCONSISTENCY_MEASURE_ALGORITHMS_constants_h
#define INCONSISTENCY_MEASURE_ALGORITHMS_constants_h
namespace constants {
    static const std::string TRUTH_VALUE_T = "tv_t";
    static const std::string TRUTH_VALUE_B = "tv_b";
    static const std::string TRUTH_VALUE_F = "tv_f";
    static const std::string TRUTH_VALUE_FORGET_T = "tv_ft";
    static const std::string TRUTH_VALUE_FORGET_F = "tv_ff";

    static const std::string TRUTH_VALUE_PREDICATE = "truthValue";
    static const std::string TRUTH_VALUE_PREDICATE_ATOM = "atomTruthValue";
    static const std::string TRUTH_VALUE_PREDICATE_INTERPRETATION = "truthValueInt";
    static const std::string TRUTH_VALUE_PREDICATE_KB_MEMBER = "truthValueKbMember";

    static const std::string ATOM = "atom";
    static const std::string ATOM_OCC = "atomOcc";
    static const std::string KB_MEMBER = "kbMember";
    static const std::string FORMULA_PREFIX = "phi_";
    static const std::string LABEL = "label";
    static const std::string INTERPRETATION = "interpretation";
    static const std::string INTERPRETATION_REQUIRED = "interpRequired";

    static const std::string CONJUNCTION = "conjunction";
    static const std::string CONJUNCT_OF = "conjunctOf";
    static const std::string NUM_CONJUNCTS = "numConjuncts";

    static const std::string DISJUNCTION = "disjunction";
    static const std::string DISJUNCT_OF = "disjunctOf";
    static const std::string NUM_DISJUNCTS = "numDisjuncts";

    static const std::string NEGATION = "negation";

    static const std::string FORMULA_IS_ATOM = "formulaIsAtom";
    static const std::string FORMULA_IS_ATOM_OCC = "formulaIsAtomOcc";

    static const std::string DIFF = "diff";
    static const std::string DALAL = "d";
    static const std::string DALAL_MAX = "dMax";
    static const std::string DALAL_SUM = "dSum";
	
	//BEGIN: Constants used in LTL measures specifically
	static const std::string IS_NEXT = "next";
    static const std::string IS_UNTIL = "until";
    static const std::string IS_GLOBALLY = "globally";
    static const std::string IS_FINALLY = "finally";
    static const std::string IS_STATE = "state";
    static const std::string FINAL_STATE = "finalState";
    static const std::string NUM_B_IN_STATE = "numBInState";
    static const std::string SUM_B = "sumB";
    static const std::string AFFECTED_STATE = "as";
	// This is also LTL constants (are they needed for ltl inc measurement though?)
    static const std::string IS_TRUE = "true";
    static const std::string IN_TRACE = "inTrace";
    static const std::string ATOM_IN_FORMULA_LTL = "aInF";
    static const std::string INTERSECTION_SIZE = "interSize";
    static const std::string TRACE = "trace";
	//END: Constants used in LTL measures 
	
	//BEGIN: Constants used for mv and problematic measures specifically
	static const std::string ATOM_IN_FORMULA_MV = "atomInFormula";
    static const std::string NUM_KB_ELEMENTS = "numKbElements";
    static const std::string QUERY_ATOM = "queryAtom";
    static const std::string IN_CS = "inCs";
    static const std::string ATOM_IN_CS = "atomInCs";
    static const std::string NUM_ELEMENTS_IN_CS = "numElementsInCs";
    static const std::string NUM_ELEMENTS_IN_SUBSET = "numElementsInSubset";
    static const std::string IS_SUBSET = "isSubset";
    static const std::string IN_SUBSET = "inSubset";
    static const std::string SUBSET_EQ = "subsetEq";
    static const std::string SUBSET_IS_SAT = "subsetIsSat";
    static const std::string IS_UNSAT = "isUnsat";

    static const std::string QUERY_ATOM_OF_SET = "queryAtomOfSet";
    static const std::string IS_CS = "isCs";
    static const std::string INTERPRETATION_OF = "interpretationOf";
    static const std::string SUBSET_OF_SET = "subsetOfSet";
    static const std::string IN_SUBSET_OF_SET = "inSubsetOfSet";
    static const std::string NUM_ATOMS_IN_CS = "numAtomsInCs";
    static const std::string BOTH_T_AND_F = "bothTandF";
    static const std::string CS_IS_UNSAT = "csIsUnsat";
    static const std::string CS_IS_MIS = "csIsMIS";
    static const std::string ATOM_IN_MIS = "atomInMIS";
    static const std::string REMOVE_F = "remove";
    static const std::string REMOVE_EQ = "removeEq";

    static const std::string IS_SUPERSET = "isSubset";
    static const std::string IN_SUPERSET = "inSubset";
    static const std::string NOT_IN_CS = "notInCs";
    static const std::string ATOM_IN_REMAINDER = "atomInRemainder";
    static const std::string CS_IS_SAT = "csIsSat";
    static const std::string NUM_SUPERSETS = "numSupersets";
    static const std::string ADDITIONAL_ELEMENT = "additionalElement";
    static const std::string SUPERSET_EQ = "supersetEq";
    static const std::string ATOM_IN_SUPERSET = "atomInSuperset";

    static const std::string QUERY_FORMULA = "queryF";
	
	static const std::string TRUTH_VALUE_PREDICATE_CS = "truthValueCS";
    static const std::string TRUTH_VALUE_PREDICATE_SET = "truthValueSet";
    // static const std::string FORMULA_IN_REMAINDER = "fInRemainder";
	//END: Constants used for mv and problematic measures
}
#endif