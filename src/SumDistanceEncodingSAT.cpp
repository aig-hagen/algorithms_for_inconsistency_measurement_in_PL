#include "InconsistencyMeasureSAT.h"
// This helper function is identical for both max-distance and sum-distance SAT encodings
Kb GetCommonDistanceEncodingForSum(const Kb &k)
{
    Kb encoding = Kb();
    if (k.GetFormulas().empty())
    {
        return encoding;
    }

    std::vector<Formula> formulas = k.GetFormulas();
    // Create copies of all formulas to represent that they
    // are all satisfiable individually and to allows us to
    // put them into relation with the "optimal interpretation"
    // later
    for (int i = 0; i < k.size(); i++)
    {
        Formula f_copy = formulas[i];
        std::set<std::string> sig = formulas[i].GetSignature();
        for (std::string p : sig)
        {
            f_copy.ReplaceAllOccurencesOfAtom(p, Formula(p + "_" + std::to_string(i)));
        }
        encoding.Add(f_copy);
    }
    // Create formulas that represent that for each formula,
    // there is a model that is either equivalent to the
    //"optimal interpretation" or becomes equivalent
    // if some of the atoms are inverted using the inverter atoms
    // The atoms of the "optimal interpretation" are represented by the "_o" atoms
    // The "inverter atoms" are the atoms with names starting with "j"
    std::set<std::string> atoms_set = k.GetSignature();
    std::vector<std::string> atoms;
    std::copy(atoms_set.begin(), atoms_set.end(), std::back_inserter(atoms));
    for (size_t i = 0; i < atoms.size(); i++)
    {
        for (int j = 0; j < k.size(); j++)
        {
            std::string name = atoms[i];
            // a => a_o || (j_ij && !a_o)
            // Formula right_pos = Formula(Type::OR, Formula(name + "_o"),
            //                             Formula(Type::AND, Formula("j" + std::to_string(i) + "_" + std::to_string(j)),
            //                                     Formula(Type::NOT, Formula(name + "_o"))));
            // Formula imp_pos = Formula(Type::IMPLIES, Formula(name + "_" + std::to_string(j)), right_pos);
            // encoding.Add(imp_pos);

            // NEW:
            // a => a_o || j_ij
            Formula right_pos = Formula(Type::OR, Formula(name + "_o"), Formula("j" + std::to_string(i) + "_" + std::to_string(j)));
            Formula imp_pos = Formula(Type::IMPLIES, Formula(name + "_" + std::to_string(j)), right_pos);
            encoding.Add(imp_pos);

            // ! a => !a_o || (j_ij && a_o)
            // Formula right_neg =
            //     Formula(Type::OR, Formula(Type::NOT, Formula(name + "_o")),
            //             Formula(Type::AND, Formula("j" + std::to_string(i) + "_" + std::to_string(j)), Formula(name + "_o")));
            // Formula imp_neg = Formula(Type::IMPLIES, Formula(Type::NOT, Formula(name + "_" + std::to_string(j))), right_neg);
            // encoding.Add(imp_neg);

            // NEW:
            // !a => !a_o || j_ij
            Formula right_neg = Formula(Type::OR, Formula(Type::NOT, Formula(name + "_o")), 
                    Formula("j" + std::to_string(i) + "_" + std::to_string(j)));
            Formula imp_neg = Formula(Type::IMPLIES, Formula(Type::NOT, Formula(name + "_" + std::to_string(j))), right_neg);
            encoding.Add(imp_neg);

        }
    }
    return encoding;
}

Kb GetSumDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc)
{
    if (u == 0)
    {
        return Kb(k);
    }
    Kb encoding = GetCommonDistanceEncodingForSum(k);

    std::set<std::string> atoms_set = k.GetSignature();
    std::vector<std::string> atoms;
    std::copy(atoms_set.begin(), atoms_set.end(), std::back_inserter(atoms));

    // Collect "inverter atoms"
    std::vector<std::string> inverters;
    for (int j = 0; j < k.size(); j++)
    {
        for (size_t i = 0; i < atoms.size(); i++)
        {
            inverters.push_back("j" + std::to_string(i) + "_" + std::to_string(j));
        }
    }

    // Cardinality constraint: At most u "inverter atoms" can be used in total
    encoding.Add(GetCardinalityConstraint(enc, inverters, u));
    return encoding;
}
