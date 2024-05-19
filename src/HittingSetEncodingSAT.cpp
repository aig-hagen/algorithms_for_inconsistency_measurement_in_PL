#include "InconsistencyMeasureSAT.h"
Kb GetHsEncoding(const Kb &k, int u, CardinalityEncoding enc)
{
    if (u == 1)
    {
        return Kb(k);
    }

    Kb encoding = Kb();
    std::vector<Formula> formulas = k.GetFormulas();
    for (int i = 0; i < u; i++)
    {
        for (int j = 0; j < k.size(); j++)
        {
            // Create copies of all formulas with renamed atoms, representing
            // the satisfiablity of formulas in different partitions
            Formula f_copy = formulas[j];
            for (std::string p : formulas[j].GetSignature())
            {
                f_copy.ReplaceAllOccurencesOfAtom(p, Formula(p + "_" + std::to_string(i)));
            }

            // Create clauses that represent membership of a formula in a block
            Formula p_ij = Formula("p_" + std::to_string(i) + "_" + std::to_string(j));
            encoding.Add(Formula(Type::IMPLIES, p_ij, f_copy));
            // Cardinality constraint
            // Every formula has to be a member of at least one partition
            Formula at_least_one = Formula(Type::OR);
            for (int xi = 0; xi < u; xi++)
            {
                Formula x_ij = Formula("p_" + std::to_string(xi) + "_" + std::to_string(j));
                at_least_one.AddSubformula(x_ij);
            }
            encoding.Add(at_least_one);
        }
    }
    return encoding;
}