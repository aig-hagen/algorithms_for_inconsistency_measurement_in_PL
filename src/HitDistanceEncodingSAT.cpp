#include "InconsistencyMeasureSAT.h"
Kb GetHitDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc)
{
    if (u == 0)
    {
        return Kb(k);
    }
    Kb encoding = Kb();
    if (u == k.size() || k.GetFormulas().empty())
    {
        return encoding;
    }

    // Add "hit variables" that represent the number of formulas
    // that are not true in an interpretation
    int hi = 0;
    std::vector<std::string> hits;
    for (Formula f : k.GetFormulas())
    {
        Formula hit = Formula("HIT_" + std::to_string(hi));
        encoding.Add(Formula(Type::OR, f, hit));
        hits.push_back("HIT_" + std::to_string(hi));
        hi++;
    }

    // Cardinality constraint: At most u "hit variables" are allowed to be true
    encoding.Add(GetCardinalityConstraint(enc, hits, u));
    return encoding;
}