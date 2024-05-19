#include <set>
#include <sstream>
#include <vector>

#include "InconsistencyMeasureSAT.h"
#include "Utils.h"

std::string GetAliasString(const Formula &f)
{
    if (f.IsAtom())
    {
        return "P" + f.GetName();
    }
    else if (f.IsAssociativeFormula() && f.size() == 1)
    {
        return GetAliasString(*(f.GetSubformulas().begin()));
    }
    else if (f.IsTautology())
    {
        return "TRUE";
    }
    else if (f.IsContradiction())
    {
        return "FALSE";
    }
    const char *Operators[] = {"FALSE", "TRUE", "N", "A", "O", "i", "I"};
    std::string alias = "P";
    auto subformulas = f.GetSubformulas();
    for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
    {
        alias += Operators[stoi(f.GetName())] + GetAliasString(*i);
    }
    return alias;
}

bool IsTrivialEquivalence(const Formula &f)
{
    if (f.IsEquivalence() && f.size() == 2)
    {
        auto subformulas = f.GetSubformulas();
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        return (left == right);
    }
    return false;
}

Kb EncodeSubformulas(const Formula &f)
{
    Kb result = Kb();
    auto subformulas = f.GetSubformulas();
    if (f.IsAtom() || !f.IsContingency() || f.empty())
    {
        return result;
    }
    else if (f.IsNegation())
    {
        Formula negated_f = Formula(*(subformulas.begin()));
        Formula pi_T = Formula(GetAliasString(negated_f) + "_T");
        Formula pi_B = Formula(GetAliasString(negated_f) + "_B");
        Formula pi_F = Formula(GetAliasString(negated_f) + "_F");
        Formula e_T = Formula(Type::IFF, Formula(GetAliasString(f) + "_T"), pi_F);
        Formula e_F = Formula(Type::IFF, Formula(GetAliasString(f) + "_F"), pi_T);
        Formula e_B = Formula(Type::IFF, Formula(GetAliasString(f) + "_B"), pi_B);
        result.Add(e_T.NaiveToCnf());
        result.Add(e_F.NaiveToCnf());
        result.Add(e_B.NaiveToCnf());
        result.Add(EncodeSubformulas(negated_f));
        return result;
    }
    else if (f.IsDisjunction())
    {
        Formula p_T = Formula(GetAliasString(f) + "_T");
        Formula p_B = Formula(GetAliasString(f) + "_B");
        Formula p_F = Formula(GetAliasString(f) + "_F");
        auto i = subformulas.begin();
        Formula first = *(i++);
        result.Add(EncodeSubformulas(first));
        if (i == subformulas.end())
        {
            Formula e_T = Formula(Type::IFF, p_T, Formula(GetAliasString(first) + "_T"));
            Formula e_F = Formula(Type::IFF, p_F, Formula(GetAliasString(first) + "_F"));
            Formula e_B = Formula(Type::IFF, p_B, Formula(GetAliasString(first) + "_B"));
            result.Add(e_T.NaiveToCnf());
            result.Add(e_F.NaiveToCnf());
            result.Add(e_B.NaiveToCnf());
            return result;
        }
        Formula rest = Formula(Type::OR);
        for (; i != subformulas.end(); ++i)
        {
            rest.AddSubformula(*i);
        }
        Formula pi_T = Formula(Type::OR, Formula(GetAliasString(first) + "_T"), Formula(GetAliasString(rest) + "_T"));
        Formula pi_F = Formula(Type::AND, Formula(GetAliasString(first) + "_F"), Formula(GetAliasString(rest) + "_F"));
        Formula pi_B = Formula(Type::AND, Formula(Type::NOT, p_T), Formula(Type::NOT, p_F));
        Formula e_T = Formula(Type::IFF, p_T, pi_T);
        Formula e_F = Formula(Type::IFF, p_F, pi_F);
        Formula e_B = Formula(Type::IFF, p_B, pi_B);
        result.Add(e_T.NaiveToCnf());
        result.Add(e_F.NaiveToCnf());
        result.Add(e_B.NaiveToCnf());
        result.Add(EncodeSubformulas(rest));
        return result;
    }
    else if (f.IsConjunction())
    {
        Formula p_T = Formula(GetAliasString(f) + "_T");
        Formula p_B = Formula(GetAliasString(f) + "_B");
        Formula p_F = Formula(GetAliasString(f) + "_F");
        auto i = subformulas.begin();
        Formula first = *(i++);
        result.Add(EncodeSubformulas(first));
        if (i == subformulas.end())
        {
            Formula e_T = Formula(Type::IFF, p_T, Formula(GetAliasString(first) + "_T"));
            Formula e_F = Formula(Type::IFF, p_F, Formula(GetAliasString(first) + "_F"));
            Formula e_B = Formula(Type::IFF, p_B, Formula(GetAliasString(first) + "_B"));
            if (!IsTrivialEquivalence(e_T))
            {
                result.Add(e_T.NaiveToCnf());
            }
            if (!IsTrivialEquivalence(e_F))
            {
                result.Add(e_F.NaiveToCnf());
            }
            if (!IsTrivialEquivalence(e_B))
            {
                result.Add(e_B.NaiveToCnf());
            }
            return result;
        }
        Formula rest = Formula(Type::AND);
        for (; i != subformulas.end(); ++i)
        {
            rest.AddSubformula((*i));
        }
        Formula pi_T = Formula(Type::AND, Formula(GetAliasString(first) + "_T"), Formula(GetAliasString(rest) + "_T"));
        Formula pi_F = Formula(Type::OR, Formula(GetAliasString(first) + "_F"), Formula(GetAliasString(rest) + "_F"));
        Formula pi_B = Formula(Type::AND, Formula(Type::NOT, p_T), Formula(Type::NOT, p_F));
        Formula e_T = Formula(Type::IFF, p_T, pi_T);
        Formula e_F = Formula(Type::IFF, p_F, pi_F);
        Formula e_B = Formula(Type::IFF, p_B, pi_B);
        result.Add(e_T.NaiveToCnf());
        result.Add(e_F.NaiveToCnf());
        result.Add(e_B.NaiveToCnf());
        result.Add(EncodeSubformulas(rest));
        return result;
    }
    else if (f.IsImplication())
    {
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj = Formula(Type::OR, Formula(Type::NOT, left), right);
        result.Add(EncodeSubformulas(disj));
        return result;
    }
    else if (f.IsEquivalence())
    {
        auto i = subformulas.begin();
        Formula left = *(i++);
        Formula right = *(i);
        Formula disj_left = Formula(Type::OR, Formula(Type::NOT, left), right);
        Formula disj_right = Formula(Type::OR, Formula(Type::NOT, right), left);
        result.Add(EncodeSubformulas(Formula(Type::AND, disj_left, disj_right)));
        return result;
    }
    else
    {
        std::stringstream msg;
        msg << "Error while generating Contension Encoding: Invalid formula " << f << std::endl;
        throw std::runtime_error(msg.str().c_str());
        return result;
    }
}

Kb GetContensionEncoding(const Kb &k, int u, CardinalityEncoding enc)
{
    if (u == 0)
    {
        return Kb(k).GetTseitinEncoding();
    }
    Kb encoding = Kb();
    if (k.GetFormulas().empty())
    {
        return encoding;
    }

    std::set<std::string> atoms_set = k.GetSignature();
    if (static_cast<int>(atoms_set.size()) == u)
    {
        return encoding;
    }
    std::vector<std::string> atoms;
    std::copy(atoms_set.begin(), atoms_set.end(), std::back_inserter(atoms));

    // Add additional truth value
    std::vector<std::string> b_atoms;
    for (std::string a : atoms)
    {
        std::string name = "P";
        b_atoms.push_back(name + a + "_B");
        Formula a_T = Formula(name + a + "_T");
        Formula a_B = Formula(name + a + "_B");
        Formula a_F = Formula(name + a + "_F");
        Formula or1 = Formula(Type::OR, a_T, a_B);
        or1.AddSubformula(a_F);
        Formula or2 = Formula(Type::OR, Formula(Type::NOT, a_T), Formula(Type::NOT, a_B));
        Formula or3 = Formula(Type::OR, Formula(Type::NOT, a_T), Formula(Type::NOT, a_F));
        Formula or4 = Formula(Type::OR, Formula(Type::NOT, a_B), Formula(Type::NOT, a_F));
        Formula conj = Formula(Type::AND, or1, or2);
        conj.AddSubformula(or3);
        conj.AddSubformula(or4);
        encoding.Add(conj.NaiveToCnf());
    }

    // Encode subformulas
    for (Formula f : k.GetFormulas())
    {
        encoding.Add(EncodeSubformulas(f));
        if (f.IsImplication())
        {
            auto subformulas = f.GetSubformulas();
            auto i = subformulas.begin();
            Formula left = *(i++);
            Formula right = *(i);
            std::string name = GetAliasString(Formula(Type::OR, Formula(Type::NOT, left), right));
            encoding.Add(Formula(Type::OR, name + "_T", name + "_B"));
        }
        else if (f.IsEquivalence())
        {
            auto subformulas = f.GetSubformulas();
            auto i = subformulas.begin();
            Formula left = *(i++);
            Formula right = *(i);
            std::string name = GetAliasString(Formula(Type::AND, Formula(Type::OR, Formula(Type::NOT, left), right),
                                                      Formula(Type::OR, Formula(Type::NOT, right), left)));
            encoding.Add(Formula(Type::OR, name + "_T", name + "_B"));
        }
        else
        {
            encoding.Add(Formula(Type::OR, Formula(GetAliasString(f) + "_T"), Formula(GetAliasString(f) + "_B")));
        }
    }

    // Cardinality constraint
    encoding.Add(GetCardinalityConstraint(enc, b_atoms, u));
    return encoding;
}
