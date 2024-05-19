#include "Kb.h"

#include <iostream>

#include "Utils.h"

Kb::Kb(const Kb &a)
{
    auto formulas = a.GetFormulas();
    for (auto i = formulas.begin(); i != formulas.end(); ++i)
    {
        this->formulas_.emplace_back(std::make_unique<Formula>(*i));
    }
}

Kb::Kb(const Formula &f)
{
    this->formulas_.emplace_back(std::make_unique<Formula>(f));
}

Kb::Kb(const std::vector<Formula> &formulas)
{
    for (Formula f : formulas)
    {
        this->formulas_.emplace_back(std::make_unique<Formula>(f));
    }
}

std::vector<Formula> Kb::GetFormulas() const
{
    std::vector<Formula> result;
    for (auto i = this->formulas_.begin(); i != this->formulas_.end(); ++i)
    {
        result.emplace_back(**i);
    }
    return result;
}

std::set<std::string> Kb::GetSignature() const
{
    std::set<std::string> sig;
    for (auto i = this->formulas_.begin(); i != this->formulas_.end(); ++i)
    {
        std::set<std::string> formula_sig = (**i).GetSignature();
        sig.insert(formula_sig.begin(), formula_sig.end());
    }
    return sig;
}

int Kb::GetNumberOfAtomOccurences() const
{
    int o = 0;
    for (auto i = this->formulas_.begin(); i != this->formulas_.end(); ++i)
    {
        o += (**i).GetNumberOfAtomOccurences();
    }
    return o;
}

void Kb::Add(const Formula &f)
{
    this->formulas_.emplace_back(std::make_unique<Formula>(f));
}

void Kb::Add(const Kb &other)
{
    auto formulas = other.GetFormulas();
    for (auto i = formulas.begin(); i != formulas.end(); ++i)
    {
        this->formulas_.emplace_back(std::make_unique<Formula>(*i));
    }
}

Formula Kb::GetTseitinEncoding(const Formula &f, int &t) const
{
    Formula cnf = Formula(Type::AND);
    Formula top_atom = Formula("t" + std::to_string(t));
    if (f.IsAtom() || !f.IsContingency())
    {
        Formula equv_left = Formula(Type::IFF, top_atom, f);
        cnf.AddSubformula(equv_left.NaiveToCnf());
        return cnf;
    }
    Type op = StringToType(f.GetName());
    Formula equv_right = Formula(op);
    auto subformulas = f.GetSubformulas();
    for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
    {
        Formula temp = Formula(*i);
        if (temp.IsLiteral() || !temp.IsContingency())
        {
            equv_right.AddSubformula(temp);
        }
        else
        {
            t += 1;
            Formula aux_atom = Formula("t" + std::to_string(t));
            equv_right.AddSubformula(aux_atom);
            Formula x = GetTseitinEncoding(temp, t);
            cnf.AddSubformula(x);
        }
    }
    Formula equv_left = Formula(Type::IFF, top_atom, equv_right);
    cnf.AddSubformula(equv_left.NaiveToCnf());
    return cnf;
}

Formula Kb::GetTseitinEncoding() const
{
    int *t = new int();
    *t = 0;
    Formula cnf = Formula(Type::AND);
    for (auto j = this->formulas_.begin(); j != this->formulas_.end(); ++j)
    {
        Formula f = Formula(**j);
        if (f.IsAtom() || !f.IsContingency())
        {
            cnf.AddSubformula(f);
            continue;
        }
        auto subformulas = f.GetSubformulas();
        *t += 1;
        Formula top_atom = Formula("t" + std::to_string(*t));
        cnf.AddSubformula(top_atom);
        Type op = StringToType(f.GetName());
        Formula equv_right = Formula(op);
        for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
        {
            Formula temp = Formula(*i);
            if (temp.IsLiteral() || !temp.IsContingency())
            {
                equv_right.AddSubformula(temp);
            }
            else
            {
                *t += 1;
                Formula aux_atom = Formula("t" + std::to_string(*t));
                equv_right.AddSubformula(aux_atom);
                Formula x = GetTseitinEncoding(temp, *t);
                cnf.AddSubformula(x);
            }
        }
        Formula equv_left = Formula(Type::IFF, top_atom, equv_right);
        cnf.AddSubformula(equv_left.NaiveToCnf());
    }
    return cnf;
}

std::ostream &operator<<(std::ostream &strm, const Kb &a)
{
    if (a.formulas_.empty())
    {
        return strm << "{}";
    }

    strm << "{";
    for (auto i = a.formulas_.begin(); i != a.formulas_.end(); ++i)
    {
        if ((i != a.formulas_.end()) && (i + 1 == a.formulas_.end()))
        {
            strm << **i;
        }
        else
        {
            strm << (**i) << ",";
        }
    }
    return strm << "}";
}

bool operator==(const Kb &a, const Kb &b)
{
    for (auto i = a.formulas_.begin(); i != a.formulas_.end(); ++i)
    {
        bool found = false;
        for (auto j = b.formulas_.begin(); j != b.formulas_.end(); ++j)
        {
            if ((**i) == (**j))
            {
                found = true;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

bool operator!=(const Kb &a, const Kb &b)
{
    return !(a == b);
}
