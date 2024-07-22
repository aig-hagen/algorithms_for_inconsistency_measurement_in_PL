#include <stdio.h>

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <cctype>

#include <EvalMaxSAT.h>
#include "src/ContensionASP.h"

#include "src/InconsistencyMeasureSAT.h"
#include "src/InconsistencyMeasureASP.h"
#include "src/InconsistencyMeasureMaxSAT.h"
#include "src/InconsistencyMeasureNaiveSAT.h"
#include "src/Parser.h"



namespace time_msrs{
    int num_solver_calls = 0; // Number of times the solver is called to find the inconsistency value
    std::vector<double> encoding_times = {}; // times needed to generate SAT-Encoding/ASP-Rules
    std::vector<double> encoding_sizes = {}; // sizes in bytes of the sat encoding for each iteration
    std::vector<double> cnf_transform_times = {}; // times needed to transform SAT-Encoding into CNF
    std::vector<double> solver_times_pure = {}; // Times needed to solve a SAT-Encoding/ the ASP-Program (without preprocessing)
    std::vector<double> solver_times_total = {}; // Times needed to solve a SAT-Encoding/ the ASP-Program (with preprocessing)
}
#include "src/Utils.h"

static void PrintCmdHelp()
{
    std::cerr << "Version die inc_val, enc, cnf, solve times pure + total ausgibt" << std::endl
              << "Call im-app with 3+ arguments: ./im-app <kb_file> <measure> <method> <format> <cenc> <debug> <m>" << std::endl
              << "<file> is a path to a  knowledge base file" << std::endl
              << "<measure> is the name of an inconsistency measure" << std::endl
              << "<method> is the algorithmic approach, either 'sat', 'linsat', 'asp', 'maxsat', 'naivecpp' " << std::endl
			  << "	Not all inconsistency measures have imolemented for each method:"
              << "	List of implemented measures for each of the five modes:" << std::endl
              << "	<method> = sat: contension, hs, hitdalal, maxdalal, sumdalal, forget" << std::endl
			  << "	<method> = linsat: contension, hs, hitdalal, maxdalal, sumdalal, forget" << std::endl
			  << "	<method> = asp: contension, hs, hitdalal, maxdalal, sumdalal, forget" << std::endl
			  << "                  contension-ltl, drastic-ltl, mv, mv2, v3b, mv3, mv-mss, mv-mss2, p, p-2" << std::endl
			  << "	<method> = maxsat: contension" << std::endl
			  << "	<method> = naivecpp: forget" << std::endl
			  << "Note that the arguments <kb_file> <measure> <method> are not optional and need to be specified" << std::endl
			  << "<format> (optional) is 'dimacs' if the kb-file is in DIMACS Format or 'tweety' if it is in the Tweety format" << std::endl
              << "<cenc> (optional) defines the cardinality encoding" << std::endl
              << "	possible 'cenc' values: sequential_counter, tree, binomial" << std::endl
              << "<debug> (optional) enables additional debug outputs"
			  << "<m>: special value that only needs to be specified for measures contension-ltl and drastic-ltl"
              << std::endl;
}

//Examples for creating formula instances
void Example()
{
    Formula a = Formula("a");
    Formula b  = Formula("b");
    std::cout << "atom: " << a << std::endl;
    Formula not_a = Formula(Type::NOT, a);
    std::cout << "literal: " << not_a << std::endl;
    Formula impl = Formula(Type::IMPLIES, a, b);
    std::cout << "implication: " << impl << std::endl;
    Formula equv = Formula(Type::IFF, not_a, impl);
    std::cout << "equivalence: " << equv << std::endl;

    // Different ways to initialize associative formulas
    Formula conj1 = Formula(Type::AND); // empty conjunction
    std::cout << "empty conj1: " << conj1 << std::endl;
    Formula conj2 = Formula(Type::AND, a, not_a);
    std::cout << "conj2: " << conj2 << std::endl;
    std::vector<Formula> disjuncts = {a, not_a, impl};
    Formula disj = Formula(Type::OR, disjuncts);
    std::cout << "disj " << disj << std::endl;
    // To add more formulas to an existing associative formula,
    // use the following method:
    conj1.AddSubformula(a);
    conj1.AddSubformula(impl);
    std::cout << "non-empty conj1: " << conj1 << std::endl;

    //Creating an empty knowledge base
    Kb k = Kb();
    //Adding formulas to k
    k.Add(b);
    k.Add(conj1);
    std::cout << "k:" << k << std::endl;

    //Creating a knowledge base with a list of formulas
    Kb k2 = Kb(disjuncts);
    //Adding all formulas of another knowledge base to k2
    k2.Add(k);
    std::cout << "k2:" << k2 << std::endl;

    // Parsing formulas
    Parser p = Parser();
    Formula f1 = p.ParseFormulaFromString("c||d");
    std::cout << "parsed f1: " << f1 << std::endl;
    Kb k3 = p.ParseKbFromString("!(a&&b)\n c || b\n");
    std::cout << "parsed k3: " << k3 << std::endl;
    // Kb k4 = p.ParseKbFromFile("/path/to/file");

    // Iterating over subformulas and doing things based on the type of formula
    Formula f2 = Formula(Type::AND, a, Formula(Type::NOT, b));
    f2.AddSubformula(f1);
    std::vector<Formula> subformulas = f2.GetSubformulas();
    for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
    {
        Formula s = (*i);
        if (s.IsLiteral())
        {
            std::cout << "subformula " << s << " is a literal" << std::endl;
        }
        if (s.IsAtom())
        {
            std::cout << "subformula " << s << " is an atom named " << s.GetName() << std::endl;
        }
        if (s.IsNegation())
        {
            std::cout << "subformula " << s << " is a negation with the inner formula " << *s.GetSubformulas().begin() << std::endl;
        }
        if (s.IsDisjunction())
        {
            std::cout << "subformula " << s << " is a disjunction with " << s.GetSubformulas().size() << " disjuncts"
                      << std::endl;
        }
    }

    //Replace atoms named "a" in f1 with formula "!x"
    Formula f3 = p.ParseFormulaFromString("a&&b&&a");
    Formula not_x = Formula(Type::NOT, Formula("x"));
    f3.ReplaceAllOccurencesOfAtom("a", not_x);
    std::cout << "replaced all a with !x: " << f3 << std::endl;
    Formula f4 = p.ParseFormulaFromString("a&&b&&a");
    f4.ReplaceIthOccurenceOfAtom("a", not_x, 2);
    std::cout << "replaced 2nd a with !x: " << f4 << std::endl;
}

//VL: Is this function obsolete? It doesn't seem to be used anywhere.
void print_formula_types(Formula f){
    if (f.IsLiteral()){
        std::cout << "Literal: " << f << std::endl;
    }
    else if (f.IsDisjunction()){
        std::cout << "Disjunction: " << f << std::endl;
        auto disjuncts = f.GetSubformulas();
        for(auto d: disjuncts){
            print_formula_types(d);
        }
    }
    else if (f.IsConjunction()){
        std::cout << "Conjunction: " << f << std::endl;
        auto conjuncts = f.GetSubformulas();
        for(auto c: conjuncts){
            print_formula_types(c);
        }
    }
}

void handle_disjunction(Formula d, EvalMaxSAT *solver, std::map<std::string, int> &dict, int &max_val, std::vector<int> &clause){
    if (d.IsLiteral()){
        if (d.IsNegation()){
            Formula inner_formula = Formula(*(d.GetSubformulas().begin()));
            std::string atom_name = inner_formula.GetName();
            // std::cout << atom_name << std::endl;
            if (dict.find(atom_name) == dict.end()) {
                // add variable to dict:
                dict[atom_name] = max_val;
                solver->newVar(max_val);
                // add to clause
                clause.push_back(-max_val);
                max_val++;
            } else {
                // solver->addClause({-dict[atom_name]});
                clause.push_back(-dict[atom_name]);
            }
        }
        else{
            // add variable to dict:
            std::string atom_name = d.GetName();
            // std::cout << atom_name << std::endl;
            if (dict.find(atom_name) == dict.end()) {
                // add variable to dict:
                dict[atom_name] = max_val;
                solver->newVar(max_val);

                // add to solver:
                // solver->addClause({max_val});
                clause.push_back(max_val);
                max_val++;
            } else {
                // solver->addClause({dict[atom_name]});
                clause.push_back(dict[atom_name]);
            }
        }
    }
    else if(d.IsDisjunction()){
        auto disjuncts = d.GetSubformulas();
        for (auto d2 : disjuncts){
            handle_disjunction(d2, solver, dict, max_val, clause);
        }
    }
    else{
        std::cout << "Something might be missing." << std::endl;
    }
}

void add_clauses_to_solver(Formula f, EvalMaxSAT *solver, std::map<std::string, int> &dict, int &max_val){
    if (f.IsLiteral()){
        std::vector<int> clause;
        if (f.IsNegation()){
            Formula inner_formula = Formula(*(f.GetSubformulas().begin()));
            std::string atom_name = inner_formula.GetName();
            // std::cout << atom_name << std::endl;
            if (dict.find(atom_name) == dict.end()) {
                // add variable to dict:
                dict[atom_name] = max_val;
                solver->newVar(max_val);

                // add to solver:
                solver->addClause({-max_val});
                max_val++;
            } else {
                solver->addClause({-dict[atom_name]});
            }
        }
        else{
            // add variable to dict:
            std::string atom_name = f.GetName();
            // std::cout << atom_name << std::endl;
            if (dict.find(atom_name) == dict.end()) {
                // add variable to dict:
                dict[atom_name] = max_val;
                solver->newVar(max_val);

                // add to solver:
                solver->addClause({max_val});
                max_val++;
            } else {
                solver->addClause({dict[atom_name]});
            }
        }
    }
    else if (f.IsDisjunction()){
        // std::cout << "Disjunction: " << f << std::endl;
        auto disjuncts = f.GetSubformulas();
        std::vector<int> clause;
        for(auto d: disjuncts){
            handle_disjunction(d, solver, dict, max_val, clause);
        }
        solver->addClause(clause);
    }
    else if (f.IsConjunction()){
        // std::cout << "Conjunction: " << f << std::endl;
        auto conjuncts = f.GetSubformulas();
        for(auto c: conjuncts){
            add_clauses_to_solver(c, solver, dict, max_val);
        }
    }
}


double get_inconsistency_value_MaxSAT(const Kb &k, ImSettings &config){
    // get hard clauses
    Kb hard_clauses = GetContensionEncodingMaxSATHardClauses(k);

    // get soft clauses
    Kb soft_clauses = GetContensionEncodingMaxSATSoftClauses(k);

    // create MaxSAT solver instance:
    auto solver = new EvalMaxSAT();

    std::map<std::string, int> dict;
    int max_val = 1;

    // add soft clauses to solver
    for (auto f: soft_clauses.GetFormulas()){
        // std::cout << f << std::endl;

        if (f.IsLiteral()){
            // check if negation or not:
            if(f.IsNegation()){
                Formula inner_formula = Formula(*(f.GetSubformulas().begin()));
                std::string atom_name = inner_formula.GetName();
                // std::cout << atom_name << std::endl;

                // add variable to dict:
                dict[atom_name] = max_val;
                solver->newVar(max_val);

                // add to solver:
                solver->addWeightedClause({-max_val},1);

                max_val++;
            }
            else{
                // add variable to dict:
                std::string atom_name = f.GetName();
                // std::cout << atom_name << std::endl;
                dict[atom_name] = max_val;
                solver->newVar(max_val);

                // add to solver:
                solver->addWeightedClause({max_val},1);

                max_val++;
            }
        }
        else{
            std::cout << "Something went wrong" << std::endl;
        }
    }

    // add hard clauses to solver:
    for (auto f: hard_clauses.GetFormulas()){
        add_clauses_to_solver(f, solver, dict, max_val);
    }

    // std::cout << "\nI'm now trying to solve!" << std::endl;
    if(!solver->solve()) {
        std::cout << "s UNSATISFIABLE" << std::endl;
    }
    // std::cout << "s OPTIMUM FOUND" << std::endl;
    // std::cout << "o " << solver->getCost() << std::endl;

    auto result = solver->getCost();

    return result;
}

int main(int argc, char *argv[])
{
    //Step 1: Checking if im_app has been called with a valid combination of arguments
	if (argc < 4)
    {
        PrintCmdHelp();
        return -1;
    }
    std::string measure = argv[2];
    std::transform(measure.begin(), measure.end(), measure.begin(), ::tolower);
    std::set<std::string> measures = {"contension", "hs", "hitdalal", "maxdalal", "sumdalal", "forget", "contension-ltl", "drastic-ltl",
	                                  "mv", "mv2", "mv3", "mv3b", "mv-mss", "mv-mss2", "mv-mus", "p", "p-2", "p-mus"};
    if (measures.find(measure) == measures.end())
    {
        std::cerr << "Error: " << measure << " is not a valid inconsistency measure" << std::endl;
        return -1;
    }

    std::string method = argv[3];
     std::transform(measure.begin(), measure.end(), measure.begin(), ::tolower);
    if (method != "sat" && method != "asp" && method != "linsat" && method != "maxsat" && method != "naivecpp")
    {
        std::cerr << "Error: " << method << " is not a valid algorithm approach, options are 'sat', 'asp' and 'linsat'" << std::endl;
        return -1;
    }

	// special case if given <measure> is an ltl measure because these measures require the additional argument <m>
	if (measure == "contension-ltl" || measure == "drastic-ltl")
	{
	  if (argc != 8)
	  {
		std::cerr << "Error: " << measure << " needs additional parameter m" << std::endl;
        return -1;
	  }

	}

	// Default cardinality encoding is the sequential counter
    CardinalityEncoding enc = CardinalityEncoding::SEQUENTIAL_COUNTER;
    bool enable_debug_info = false;
    if (argc>5)
    {
      std::string cenc = argv[5];
      if (isdigit(cenc[0]))
      {
        enable_debug_info = (std::stoi(argv[5])==1);
      }
      else
      {
        if (cenc == "sequential_counter")
        {
          enc = CardinalityEncoding::SEQUENTIAL_COUNTER;
        }
        else if (cenc == "tree")
        {
          enc = CardinalityEncoding::TREE;
        }
        else if (cenc == "binomial")
        {
          enc = CardinalityEncoding::BINOMIAL;
        }
        if (argc>6)
        {
          enable_debug_info = (std::stoi(argv[6])==1);
        }
      }
    }

    if (enable_debug_info)
    {
      std::cout << "Success: Using inconsistency measure: " << measure << std::endl;
    }

	//Step 2: Parsing the given kb-file depending on its format (either tweety or dimacs)
    std::string file = argv[1];
    Parser p = Parser();
	Kb k = Kb();

	if(argc > 4) // in case the format of the kb-file has been explicitly specified
    {
      std::string format = argv[4];
	  if(format == "tweety")
      {
		k.Add(p.ParseKbFromFile(file));
	  }
	  else if(format == "dimacs")
	  {
		k.Add(p.ParseKbFromDIMACSFile(file));
	  }
	  else
	  {
		std::cerr << "Error: " << format << "is not a valid knowledge base format (choose either tweety or dimacs)" << std::endl;
        return -1;
	  }
    }
    else // default format is Tweety Format for knowledge bases
    {
	  k.Add(p.ParseKbFromFile(file));
	}

    if (enable_debug_info)
    {
      std::cout << "Success: Parsed KB: " << file << std::endl;
      std::cout << k << std::endl;
    }

	//Step 3: Preparing the configuration
    ImSettings config;
    config.measure_name = measure;
    config.offset = 0;
    config.is_in_cnf = false;
    config.cardinality_encoding = enc;

	//Step 4: Computing the inconsistency value
    double result {};
    if (method == "asp")
    {
        result = get_inconsistency_value(k, config);

    }
    else if (method == "sat")
    {
        result = GetInconsistencyValue(k, config);
    }
    else if (method == "linsat")
    {
        result = GetInconsistencyValueLinSearch(k, config);
    }
	else if(method == "maxsat")
    {
        result = get_inconsistency_value_MaxSAT(k, config);
    }
	else if(method == "naivecpp")
    {
        result = get_inconsistency_naive_value(k, config);
    }

	//Step 5: Printing the result
    std::string result_string = std::to_string(result);
    if (result == -1)
    {
      result_string = "INF";
    }

    if (enable_debug_info)
    {
      std::cout << "Success: Found inconsistency value: " << std::endl;
    }
    // compute avg time spent encoding, transforming the encoding to cnf and solving
    double avg_encode_time = 0.0;
    for(int i = 0; i < time_msrs::encoding_times.size(); i++)
        avg_encode_time += time_msrs::encoding_times.at(i);
    if(time_msrs::encoding_times.size() != 0)
        avg_encode_time /= time_msrs::encoding_times.size();

    double avg_cnf_transform_time = 0.0;
    for(int i = 0; i < time_msrs::cnf_transform_times.size(); i++)
        avg_cnf_transform_time += time_msrs::cnf_transform_times.at(i);
    if(time_msrs::cnf_transform_times.size() != 0)
        avg_cnf_transform_time /= time_msrs::cnf_transform_times.size();

    double avg_solve_time_pure = 0.0;
    for(int i = 0; i < time_msrs::solver_times_pure.size(); i++)
        avg_solve_time_pure += time_msrs::solver_times_pure.at(i);
    if(time_msrs::solver_times_pure.size() != 0)
        avg_solve_time_pure /= time_msrs::solver_times_pure.size();

    double avg_solve_time_total = 0.0;
    for(int i = 0; i < time_msrs::solver_times_total.size(); i++)
        avg_solve_time_total += time_msrs::solver_times_total.at(i);
    if(time_msrs::solver_times_total.size() != 0)
        avg_solve_time_total /= time_msrs::solver_times_total.size();



    // We don't do encoding sizes for now
    /*
    // find min max avg and variance of encoding_sizes of all binary-search iterations
    double min_enc_size = time_msrs::encoding_sizes.at(0);
    double max_enc_size = time_msrs::encoding_sizes.at(0);
    double avg_enc_size = 0.0;
    for(int i = 0; i < time_msrs::encoding_sizes.size(); i++){
        avg_enc_size += time_msrs::encoding_sizes.at(i);
        if (time_msrs::encoding_sizes.at(i) < min_enc_size)
            min_enc_size = time_msrs::encoding_sizes.at(i);
        if (time_msrs::encoding_sizes.at(i) > max_enc_size)
            max_enc_size = time_msrs::encoding_sizes.at(i);
    }
    avg_enc_size /= time_msrs::encoding_sizes.size();

    // compute variance of encoding size
    double enc_size_variance = 0.0;
    for(int i = 0; i < time_msrs::encoding_sizes.size(); i++)
        enc_size_variance += (time_msrs::encoding_sizes.at(i) - avg_enc_size) * (time_msrs::encoding_sizes.at(i) - avg_enc_size);
    enc_size_variance /= time_msrs::encoding_sizes.size();
    */

	// this block is for bugfixing version
	/*std::cout << "This is a Test-Version" << std::endl;
	std::cout << "argc = " << argc << std::endl;
	for(int i = 0; i < argc; i++){
		std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
	}
	std::cout << "Parsed Knowledge Base Contents:" << std::endl;
	std::cout << k << std::endl;
	std::cout << "Constructed Knowledge Base Contents:" << std::endl;
	Formula a = Formula("a");
    Formula b  = Formula("b");
	//Creating an empty knowledge base
    Kb test_kb = Kb();
    //Adding formulas to k
	test_kb.Add(a);
    test_kb.Add(b);
    std::cout << "test_kb:" << test_kb << std::endl;
    // print result_string that has the following format:
    // "<inc_value>-<num_solver_calls>-<avg_encoding_time>-<avg_cnf_transform_time>-<avg_solver_time>"
	*/

    std::cout.precision(17);
    std::cout << result_string << "_"
              << time_msrs::num_solver_calls << "_"
              << avg_encode_time << "_"
              << avg_cnf_transform_time << "_"
              << avg_solve_time_pure << "_"
              << avg_solve_time_total
              //<< min_enc_size << "-"
              //<< max_enc_size << "-"
              //<< avg_enc_size << "-"
              //<< enc_size_variance
              << std::endl;
    /*
    std::cout << "length of encoding_sizes: " << time_msrs::encoding_sizes.size() << std::endl;
    for (int i = 0; i < time_msrs::encoding_sizes.size(); i++){
        std::cout << "Encoding Size at " << i << " is: " << time_msrs::encoding_sizes.at(i) << std::endl;
    }
    */
    return 0;
}
