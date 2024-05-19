#include <chrono>

#include "InconsistencyMeasureMaxSAT.h"
// #include "SatSolver.h"
#include "Utils.h"

// #include <pstream.h>

#include <iostream>
// TODO:
// - differentiate between soft and hard clauses
// - create MaxSAT solver class
// - create encoding from hard and soft clauses
// - pipe encoding to MaxSAT solver
// - parse result 



// double BinarySearchInconsistencyValue(const Kb &k, int max,
//                                       std::function<Kb(const Kb &k, int u, CardinalityEncoding enc)> GetSatEncoding,
//                                       const ImSettings &config)
// {
//     int offset = config.offset;
//     bool is_in_cnf = config.is_in_cnf;
//     CardinalityEncoding enc = config.cardinality_encoding;

//     int inc_val = -1;

//     int min = 0 + offset; 
//     SatSolver solver = SatSolver();
//     while (min <= max)
//     {
//         int mid = min + (max - min) / 2;
//         // measure the time it takes to generate the SAT-Encoding
//         auto encoding_start = std::chrono::steady_clock::now();
//         Kb encoding = GetSatEncoding(k, mid, enc);
//         auto encoding_end = std::chrono::steady_clock::now();
//         double interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
//                           / double (1000000.0);
//         time_msrs::encoding_times.push_back(interval);

//         if (solver.IsSatisfiable(encoding, is_in_cnf))
//         {
//             if (inc_val < 0 || (mid - offset) < inc_val)
//                 inc_val = mid - offset;
//             max = mid - 1; 
//         }
//         else
//         {
//             min = mid + 1;
//         }
//     }

//     return inc_val;
// }

//VL: Is this function obsolete? It doesn't seem to be used anywhere.
double SearchInconsistencyValueMaxSAT(const Kb &k, 
                                std::function<Kb(const Kb &k)> GetContensionEncodingMaxSATHardClauses,
                                std::function<Kb(const Kb &k)> GetContensionEncodingMaxSATSoftClauses)
{
    // get hard clauses:
    Kb hard_clauses = GetContensionEncodingMaxSATHardClauses(k);
    Kb soft_clauses = GetContensionEncodingMaxSATSoftClauses(k);
    // maybe check if encodings look correct?

    // TODO: get input string for solver

    // TEST:
    std::string test_str = "";
    test_str.append("p wcnf 3 5 4\n");
    test_str.append("1 1 0\n");
    test_str.append("1 2 0\n");
    test_str.append("1 3 0\n");
    test_str.append("4 -1 -2 0\n");
    test_str.append("4 -2 -3 0\n");

    std::string test = "p wcnf 3 5 4\n1 1 0\n1 2 0\n1 3 0\n4 -1 -2 0\n4 -2 -3 0";

    // how to use an external program:
    // int status = system("./foo 1 2 3");
    // int status = system("./cashwmaxsatplus");
    // std::cout << status << std::endl;
    std::string solver_path = "../lib/Cashwmaxsat-Plus/bin/cashwmaxsatplus";
    // char* solver_path = "../lib/Cashwmaxsat-Plus/bin/cashwmaxsatplus";

    // redi::pstream proc("/home/isabelle/Schreibtisch/Uni/AIM/MaxSAT/max-sat-tests/lib/Cashwmaxsat-Plus/bin/cashwmaxsatplus", redi::pstreams::pstdout | redi::pstreams::pstdin | redi::pstreams::pstderr);
    // redi::pstream proc("bla", redi::pstreams::pstdout | redi::pstreams::pstdin | redi::pstreams::pstderr);
    // proc << test;
    // proc << redi::peof;

    // // std::cout << "TEST" << std::endl;

    // std::string line;
    // while (std::getline(proc,line)) {
    //     // std::cout << "hi" << std::endl;
    //     std::cout << line << std::endl;
    // }

    // TODO: now pass clauses to solver
    //  -> add weights somehow
    // TODO: then parse result 

    return 1234.0;
}

//VL: Is this function obsolete? It doesn't seem to be used anywhere.
double GetInconsistencyValueMaxSAT(const Kb &k, ImSettings &config)
{
    if (config.measure_name == "contension")
    {
        if (k.size() == 0)
        {
            return 0;
        }
        int max = (k.GetSignature()).size();
        config.is_in_cnf = true;
        // return BinarySearchInconsistencyValue(k, max, GetContensionEncoding, config);
        return SearchInconsistencyValueMaxSAT(k, GetContensionEncodingMaxSATHardClauses, GetContensionEncodingMaxSATSoftClauses);
    }

    // else if (config.measure_name == "forget")
    // {
    //     int max = k.GetNumberOfAtomOccurences();
    //     return BinarySearchInconsistencyValue(k, max, GetForgettingEncoding, config);
    // }
    // else if (config.measure_name == "hs")
    // {
    //     if (k.size() == 0)
    //     {
    //         return 0;
    //     }
    //     int max = k.size();
    //     config.offset = 1;
    //     return BinarySearchInconsistencyValue(k, max, GetHsEncoding, config);
    // }
    // else if (config.measure_name == "hitdalal")
    // {
    //     int max = k.size();
    //     int value = BinarySearchInconsistencyValue(k, max, GetHitDistanceEncoding, config);
    //     if (value == -1)
    //     {
    //         return max;
    //     }
    //     else
    //     {
    //         return value;
    //     }
    // }
    // else if (config.measure_name == "maxdalal")
    // {
    //     int max = (k.GetSignature()).size();
    //     return BinarySearchInconsistencyValue(k, max, GetMaxDistanceEncoding, config);
    // }
    // else if (config.measure_name == "sumdalal")
    // {
    //     int max = k.size() * (k.GetSignature()).size();
    //     return BinarySearchInconsistencyValue(k, max, GetSumDistanceEncoding, config);
    // }
    else
    {
        throw std::runtime_error("Unknown/unimplemented inconsistency measure");
    }
	return 0;
}

// Kb GetHsEncoding(const Kb &k, int u, CardinalityEncoding enc)
// {
//     if (u == 1)
//     {
//         return Kb(k);
//     }

//     Kb encoding = Kb();
//     std::vector<Formula> formulas = k.GetFormulas();
//     for (int i = 0; i < u; i++)
//     {
//         for (int j = 0; j < k.size(); j++)
//         {
//             // Create copies of all formulas with renamed atoms, representing
//             // the satisfiablity of formulas in different partitions
//             Formula f_copy = formulas[j];
//             for (std::string p : formulas[j].GetSignature())
//             {
//                 f_copy.ReplaceAllOccurencesOfAtom(p, Formula(p + "_" + std::to_string(i)));
//             }

//             // Create clauses that represent membership of a formula in a block
//             Formula p_ij = Formula("p_" + std::to_string(i) + "_" + std::to_string(j));
//             encoding.Add(Formula(Type::IMPLIES, p_ij, f_copy));
//             // Cardinality constraint
//             // Every formula has to be a member of at least one partition
//             Formula at_least_one = Formula(Type::OR);
//             for (int xi = 0; xi < u; xi++)
//             {
//                 Formula x_ij = Formula("p_" + std::to_string(xi) + "_" + std::to_string(j));
//                 at_least_one.AddSubformula(x_ij);
//             }
//             encoding.Add(at_least_one);
//         }
//     }
//     return encoding;
// }

// Kb GetHitDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc)
// {
//     if (u == 0)
//     {
//         return Kb(k);
//     }
//     Kb encoding = Kb();
//     if (u == k.size() || k.GetFormulas().empty())
//     {
//         return encoding;
//     }

//     // Add "hit variables" that represent the number of formulas
//     // that are not true in an interpretation
//     int hi = 0;
//     std::vector<std::string> hits;
//     for (Formula f : k.GetFormulas())
//     {
//         Formula hit = Formula("HIT_" + std::to_string(hi));
//         encoding.Add(Formula(Type::OR, f, hit));
//         hits.push_back("HIT_" + std::to_string(hi));
//         hi++;
//     }

//     // Cardinality constraint: At most u "hit variables" are allowed to be true
//     encoding.Add(GetCardinalityConstraint(enc, hits, u));
//     return encoding;
// }

// // The part of the encoding shared by both max-distance and sum-distance
// Kb GetCommonDistanceEncoding(const Kb &k)
// {
//     Kb encoding = Kb();
//     if (k.GetFormulas().empty())
//     {
//         return encoding;
//     }

//     std::vector<Formula> formulas = k.GetFormulas();
//     // Create copies of all formulas to represent that they
//     // are all satisfiable individually and to allows us to
//     // put them into relation with the "optimal interpretation"
//     // later
//     for (int i = 0; i < k.size(); i++)
//     {
//         Formula f_copy = formulas[i];
//         std::set<std::string> sig = formulas[i].GetSignature();
//         for (std::string p : sig)
//         {
//             f_copy.ReplaceAllOccurencesOfAtom(p, Formula(p + "_" + std::to_string(i)));
//         }
//         encoding.Add(f_copy);
//     }
//     // Create formulas that represent that for each formula,
//     // there is a model that is either equivalent to the
//     //"optimal interpretation" or becomes equivalent
//     // if some of the atoms are inverted using the inverter atoms
//     // The atoms of the "optimal interpretation" are represented by the "_o" atoms
//     // The "inverter atoms" are the atoms with names starting with "j"
//     std::set<std::string> atoms_set = k.GetSignature();
//     std::vector<std::string> atoms;
//     std::copy(atoms_set.begin(), atoms_set.end(), std::back_inserter(atoms));
//     for (size_t i = 0; i < atoms.size(); i++)
//     {
//         for (int j = 0; j < k.size(); j++)
//         {
//             std::string name = atoms[i];
//             // a => a_o || (j_ij && !a_o)
//             // Formula right_pos = Formula(Type::OR, Formula(name + "_o"),
//             //                             Formula(Type::AND, Formula("j" + std::to_string(i) + "_" + std::to_string(j)),
//             //                                     Formula(Type::NOT, Formula(name + "_o"))));
//             // Formula imp_pos = Formula(Type::IMPLIES, Formula(name + "_" + std::to_string(j)), right_pos);
//             // encoding.Add(imp_pos);

//             // NEW:
//             // a => a_o || j_ij
//             Formula right_pos = Formula(Type::OR, Formula(name + "_o"), Formula("j" + std::to_string(i) + "_" + std::to_string(j)));
//             Formula imp_pos = Formula(Type::IMPLIES, Formula(name + "_" + std::to_string(j)), right_pos);
//             encoding.Add(imp_pos);

//             // ! a => !a_o || (j_ij && a_o)
//             // Formula right_neg =
//             //     Formula(Type::OR, Formula(Type::NOT, Formula(name + "_o")),
//             //             Formula(Type::AND, Formula("j" + std::to_string(i) + "_" + std::to_string(j)), Formula(name + "_o")));
//             // Formula imp_neg = Formula(Type::IMPLIES, Formula(Type::NOT, Formula(name + "_" + std::to_string(j))), right_neg);
//             // encoding.Add(imp_neg);

//             // NEW:
//             // !a => !a_o || j_ij
//             Formula right_neg = Formula(Type::OR, Formula(Type::NOT, Formula(name + "_o")), 
//                     Formula("j" + std::to_string(i) + "_" + std::to_string(j)));
//             Formula imp_neg = Formula(Type::IMPLIES, Formula(Type::NOT, Formula(name + "_" + std::to_string(j))), right_neg);
//             encoding.Add(imp_neg);

//         }
//     }
//     return encoding;
// }

// Kb GetMaxDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc)
// {
//     if (u == 0)
//     {
//         return Kb(k);
//     }
//     Kb encoding = GetCommonDistanceEncoding(k);
//     std::set<std::string> atoms_set = k.GetSignature();
//     std::vector<std::string> atoms;
//     std::copy(atoms_set.begin(), atoms_set.end(), std::back_inserter(atoms));

//     // Collect "inverter atoms"
//     std::vector<std::vector<std::string>> inverters = std::vector<std::vector<std::string>>();
//     for (int j = 0; j < k.size(); j++)
//     {
//         std::vector<std::string> temp = std::vector<std::string>();
//         for (size_t i = 0; i < atoms.size(); i++)
//         {
//             temp.push_back("j" + std::to_string(i) + "_" + std::to_string(j));
//         }
//         inverters.push_back(temp);
//     }
//     int ki = 0;
//     // Cardinality constraint: At most u "inverter atoms" can be used per
//     // formula/model
//     for (std::vector<std::string> inv : inverters)
//     {
//         encoding.Add(GetCardinalityConstraintNamed(enc, inv, u, "COUNT_" + std::to_string(ki++)));
//     }
//     return encoding;
// }

// Kb GetSumDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc)
// {
//     if (u == 0)
//     {
//         return Kb(k);
//     }
//     Kb encoding = GetCommonDistanceEncoding(k);

//     std::set<std::string> atoms_set = k.GetSignature();
//     std::vector<std::string> atoms;
//     std::copy(atoms_set.begin(), atoms_set.end(), std::back_inserter(atoms));

//     // Collect "inverter atoms"
//     std::vector<std::string> inverters;
//     for (int j = 0; j < k.size(); j++)
//     {
//         for (size_t i = 0; i < atoms.size(); i++)
//         {
//             inverters.push_back("j" + std::to_string(i) + "_" + std::to_string(j));
//         }
//     }

//     // Cardinality constraint: At most u "inverter atoms" can be used in total
//     encoding.Add(GetCardinalityConstraint(enc, inverters, u));
//     return encoding;
// }
