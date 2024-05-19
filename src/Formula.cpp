#include "Formula.h"

#include <cctype>
#include <sstream>

Formula::Formula(const Formula &f)
{
    this->value_ = f.GetName();
    this->subformulas_ = f.GetSubformulasPtrs();
}

Formula::Formula(std::string atom)
{
    if (isdigit(atom[0]))
    {
        std::stringstream msg;
        msg << "Error while constructing Formula " << atom << ". Atom names need to begin with a letter";
        throw std::runtime_error(msg.str().c_str());
    }
    else
    {
        this->value_ = atom;
    }
}

Formula::Formula(Type op)
{
    this->value_ = std::to_string(op);
}

Formula::Formula(Type op, const Formula &f)
{
    this->value_ = std::to_string(op);
    if (op == 1 || op == 0)
    {
        std::stringstream msg;
        msg << "Error while constructing Formula " << TypeToString(op) << "(" << f << ")"
            << ". You cannot create a tautology or contradiction with subformulas";
        throw std::runtime_error(msg.str().c_str());
    }
    else
    {
        this->subformulas_.emplace_back(std::make_shared<Formula>(f));
    }
}

Formula::Formula(Type op, const Formula &a, const Formula &b)
{
    this->value_ = std::to_string(op);
    if (op == 1 || op == 0)
    {
        std::stringstream msg;
        msg << "Error while constructing Formula " << TypeToString(op) << "(" << a << "," << b << ")"
            << ". You cannot create a tautology or contradiction with subformulas";
        throw std::runtime_error(msg.str().c_str());
    }
    else if (op == 2)
    {
        std::stringstream msg;
        msg << "Error while constructing Formula " << TypeToString(op) << "(" << a << "," << b << ")"
            << ". You cannot create a negation with more than one inner formula";
        throw std::runtime_error(msg.str().c_str());
    }
    else
    {
        this->subformulas_.emplace_back(std::make_shared<Formula>(a));
        this->subformulas_.emplace_back(std::make_shared<Formula>(b));
    }
}

Formula::Formula(Type op, const std::vector<Formula> &formulas)
{
    this->value_ = std::to_string(op);
    if (formulas.size() > 0 && (op == 1 || op == 0))
    {
        std::stringstream msg;
        throw std::runtime_error("Error while constructing Formula: Your are "
                                 "attempting to add subformulas to a "
                                 "tautology or contradiction.");
    }
    else if (formulas.size() == 2 && op == 2)
    {
        std::stringstream msg;
        throw std::runtime_error("Error while constructing Formula: Your are "
                                 "attempting to create a negation with more "
                                 "than one inner formula.");
    }
    else if (formulas.size() > 2 && (op == 5 || op == 6))
    {
        std::stringstream msg;
        throw std::runtime_error("Error while constructing Formula: Your are "
                                 "attempting to create a formula with more "
                                 "than two subformulas that is not a "
                                 "conjunction or disjunction.");
    }
    else
    {
        for (Formula f : formulas)
        {
            this->subformulas_.emplace_back(std::make_shared<Formula>(f));
        }
        this->TrimAssociativeFormula();
    }
}

std::vector<Formula> Formula::GetSubformulas() const
{
    std::vector<Formula> result;
    for (auto i = this->subformulas_.begin(); i != this->subformulas_.end(); ++i)
    {
        result.emplace_back(**i);
    }
    return result;
}

int Formula::GetNumberOfAtomOccurences() const
{
    int o = 0;
    if (this->IsAtom())
    {
        ++o;
    }
    else
    {
        for (auto i = this->subformulas_.begin(); i != this->subformulas_.end(); ++i)
        {
            o += (**i).GetNumberOfAtomOccurences();
        }
    }
    return o;
}

int Formula::GetNumberOfAtomOccurences(std::string atom) const
{
    int o = 0;
    if (this->IsAtom() && this->value_ == atom)
    {
        ++o;
    }
    else
    {
        for (auto i = this->subformulas_.begin(); i != this->subformulas_.end(); ++i)
        {
            o += (**i).GetNumberOfAtomOccurences(atom);
        }
    }
    return o;
}

std::set<std::string> Formula::GetSignature() const
{
    std::set<std::string> sig;
    if (this->IsAtom())
    {
        sig.insert(this->value_);
    }
    else
    {
        for (auto i = this->subformulas_.begin(); i != this->subformulas_.end(); ++i)
        {
            std::set<std::string> subformula_sig = (**i).GetSignature();
            sig.insert(subformula_sig.begin(), subformula_sig.end());
        }
    }
    return sig;
}

void Formula::AddSubformula(const Formula &f)
{
    if (((this->IsNegation() || this->IsLiteral()) && !this->empty()) || !this->IsContingency())
    {
        std::stringstream msg;
        msg << "Error while adding formula " << f << " to " << *this
            << ". You cannot add a subformula to a non-empty negation, literal, "
               "tautology, or contradiction";
        throw std::runtime_error(msg.str().c_str());
    }
    else if ((this->IsImplication() || this->IsEquivalence()) && this->size() == 2)
    {
        std::stringstream msg;
        msg << "Error while adding formula " << f << " to " << *this
            << ".  You cannot add more than 2 subformulas to an implication or "
               "equivalence";
        throw std::runtime_error(msg.str().c_str());
    }
    else
    {
        this->subformulas_.emplace_back(std::make_shared<Formula>(f));
    }
}

void Formula::ReplaceSubformulas(std::vector<Formula> new_subformulas)
{
    this->subformulas_.clear();
    for (Formula f : new_subformulas)
    {
        this->subformulas_.emplace_back(std::make_shared<Formula>(f));
    }
}

void Formula::TrimAssociativeFormula()
{
    if (this->IsAssociativeFormula())
    {
        auto subformulas = this->GetSubformulas();
        Type op = Type::AND;
        if (this->IsDisjunction())
        {
            op = Type::OR;
        }
        Formula result = Formula(op);

        for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
        {
            Formula temp = Formula(*i);
            if (temp.GetName() == this->GetName())
            {
                auto subformulas2 = temp.GetSubformulas();
                for (auto j = subformulas2.begin(); j != subformulas2.end(); ++j)
                {
                    Formula it = Formula(*j);
                    result.AddSubformula(it);
                }
            }
            else
            {
                result.AddSubformula(temp);
            }
        }
        auto subs = result.GetSubformulas();
        this->subformulas_.clear();
        for (auto j = subs.begin(); j != subs.end(); ++j)
        {
            Formula it = Formula(*j);
            this->AddSubformula(it);
        }
    }
}

void Formula::GetPermutations(std::vector<std::vector<Formula>> &in, size_t i, std::vector<Formula> &accum,
                              std::vector<std::vector<Formula>> &out) const
{
    if (i == in.size())
    {
        out.emplace_back(accum);
    }
    else
    {
        std::vector<Formula> row = in[i];
        for (size_t j = 0; j < row.size(); ++j)
        {
            std::vector<Formula> temp = std::vector<Formula>(accum);
            temp.push_back(row[j]);
            GetPermutations(in, i + 1, temp, out);
        }
    }
}

Formula Formula::NaiveToCnf() const
{
    auto subformulas = this->GetSubformulas();
    if (this->IsAtom() || this->IsLiteral() || !this->IsContingency())
    {
        return Formula(*this);
    }
    else if (this->IsNegation())
    {
        Formula negated = Formula(*(subformulas.begin()));
        auto subformulas2 = negated.GetSubformulas();
        // double negation
        if (negated.IsNegation())
        {
            Formula negated_negated = Formula(*(subformulas2.begin()));
            return negated_negated.NaiveToCnf();
        }
        // de morgan
        else if (negated.IsConjunction())
        {
            Formula disj = Formula(Type::OR);
            for (auto i = subformulas2.begin(); i != subformulas2.end(); ++i)
            {
                disj.AddSubformula(Formula(Type::NOT, Formula((*i))));
            }
            return (disj.NaiveToCnf());
        }
        // de morgan
        else if (negated.IsDisjunction())
        {
            Formula conj = Formula(Type::AND);
            for (auto i = subformulas2.begin(); i != subformulas2.end(); ++i)
            {
                conj.AddSubformula(Formula(Type::NOT, Formula((*i))));
            }
            return (conj.NaiveToCnf());
        }
        else if (negated.IsTautology())
        {
            return Formula(Type::FALSE);
        }
        else if (negated.IsContradiction())
        {
            return Formula(Type::TRUE);
        }
        else
        {
            return Formula(Type::NOT, negated.NaiveToCnf()).NaiveToCnf();
        }
    }
    // single conjuncts, disjuncts, equivalences
    else if (subformulas.size() == 1)
    {
        return (*subformulas.begin()).NaiveToCnf();
    }
    else if (this->IsConjunction())
    {
        Formula conj = Formula(Type::AND);
        for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
        {
            conj.AddSubformula(Formula((*i)).NaiveToCnf());
        }
        return conj;
    }
    else if (this->IsDisjunction())
    {
        std::vector<std::vector<Formula>> conjs;
        for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
        {
            std::vector<Formula> inner_conjs;
            Formula cnf = (Formula((*i)).NaiveToCnf());
            if (cnf.IsConjunction())
            {
                auto subformulas2 = cnf.GetSubformulas();
                for (auto j = subformulas2.begin(); j != subformulas2.end(); ++j)
                {
                    inner_conjs.push_back(Formula((*j)));
                }
            }
            else
            {
                inner_conjs.push_back(cnf);
            }
            conjs.push_back(inner_conjs);
        }

        std::vector<std::vector<Formula>> combs;
        std::vector<Formula> tmp5;
        // TODO: Remove duplicates
        GetPermutations(conjs, 0, tmp5, combs);

        Formula new_conjs = Formula(Type::AND);
        for (std::vector<Formula> ci : combs)
        {
            Formula disj = Formula(Type::OR);
            for (Formula fi : ci)
            {
                disj.AddSubformula(fi);
            }
            new_conjs.AddSubformula(disj);
        }
        return new_conjs;
    }
    else if (this->IsImplication())
    {
        Formula left = Formula(*(subformulas.begin()));
        Formula right = Formula(*(subformulas.begin() + 1));
        return Formula(Type::OR, Formula(Type::NOT, left), right).NaiveToCnf();
    }
    else if (this->IsEquivalence())
    {
        Formula left = Formula(*(subformulas.begin()));
        Formula right = Formula(*(subformulas.begin() + 1));
        Formula impl1 = Formula(Type::OR, Formula(Type::NOT, left), right);
        Formula impl2 = Formula(Type::OR, Formula(Type::NOT, right), left);
        return Formula(Type::AND, impl1, impl2).NaiveToCnf();
    }
    else
    {
        std::stringstream msg;
        msg << "NaiveToCnf Error: Formula could not be converted to CNF: " << *this;
        throw std::runtime_error(msg.str().c_str());
        return Formula(*this);
    }
}

Formula Formula::ReplaceIthOccurenceOfAtomRecursive(std::string atom, const Formula &f, int i, int &j) const
{
    if (i <= 0)
    {
        return Formula(*this);
    }
    else if (this->IsAtom())
    {
        if (this->value_ == atom)
        {
            j++;
            if (i == j)
            {
                return Formula(f);
            }
        }
        return Formula(*this);
    }
    else
    {
        Formula result = Formula(StringToType(this->value_));
        for (auto si = this->subformulas_.begin(); si != this->subformulas_.end(); ++si)
        {
            Formula subresult = Formula(**si).ReplaceIthOccurenceOfAtomRecursive(atom, f, i, j);
            result.subformulas_.emplace_back(std::make_shared<Formula>(subresult));
        }
        return result;
    }
}

void Formula::ReplaceIthOccurenceOfAtom(std::string atom, const Formula &f, int i)
{
    if (i > 0)
    {
        int j = 0;
        Formula modified = ReplaceIthOccurenceOfAtomRecursive(atom, f, i, j);
        this->value_ = modified.GetName();
        auto subformulas = modified.GetSubformulas();
        this->subformulas_.clear();
        for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
        {
            Formula sub = Formula(*i);
            this->subformulas_.emplace_back(std::make_shared<Formula>(sub));
        }
    }
}

void Formula::ReplaceAllOccurencesOfAtom(std::string atom, const Formula &f)
{
    if (this->IsAtom())
    {
        if (this->value_ == atom)
        {
            this->value_ = f.GetName();
            auto subformulas = f.GetSubformulas();
            for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
            {
                Formula sub = Formula(*i);
                this->subformulas_.emplace_back(std::make_shared<Formula>(sub));
            }
        }
    }
    else
    {
        Formula result = Formula(StringToType(this->value_));
        for (auto i = this->subformulas_.begin(); i != this->subformulas_.end(); ++i)
        {
            Formula subresult = (**i);
            subresult.ReplaceAllOccurencesOfAtom(atom, f);
            result.subformulas_.emplace_back(std::make_shared<Formula>(subresult));
        }
        this->value_ = result.GetName();
        auto subformulas = result.GetSubformulas();
        this->subformulas_.clear();
        for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
        {
            Formula sub = Formula(*i);
            this->subformulas_.emplace_back(std::make_shared<Formula>(sub));
        }
    }
}

bool operator==(const Formula &a, const Formula &b)
{
    if (a.value_ != b.value_)
    {
        return false;
    }
    if (a.subformulas_.size() != b.subformulas_.size())
    {
        return false;
    }
    if (a.IsAssociativeFormula() || a.IsEquivalence())
    {
        for (auto i = a.subformulas_.begin(); i != a.subformulas_.end(); ++i)
        {
            bool found = false;
            for (auto j = b.subformulas_.begin(); j != b.subformulas_.end(); ++j)
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
    }
    else if (a.IsNegation() || a.IsImplication())
    {
        auto j = b.subformulas_.begin();
        for (auto i = a.subformulas_.begin(); i != a.subformulas_.end(); ++i, ++j)
        {
            Formula l = Formula(**i);
            Formula r = Formula(**j);
            if (l != r)
            {
                return false;
            }
        }
    }
    return true;
}

bool operator!=(const Formula &a, const Formula &b)
{
    return !(a == b);
}

std::ostream &operator<<(std::ostream &strm, const Formula &a)
{
    if (a.subformulas_.empty())
    {
        if (a.IsAtom())
        {
            return strm << a.value_;
        }
        else if (a.IsTautology())
        {
            return strm << "true";
        }
        else if (a.IsContradiction())
        {
            return strm << "false";
        }
    }

    const char *Operators[] = {"FALSE", "TRUE", "!", "&&", "||", "=>", "<=>"};

    bool no_parentheses = a.IsNegation() || (a.GetSubformulas().size() == 1);
    if (a.IsNegation())
    {
        strm << "!";
    }
    if (!no_parentheses)
    {
        strm << "(";
    }
    for (auto i = a.subformulas_.begin(); i != a.subformulas_.end(); ++i)
    {
        if ((i != a.subformulas_.end()) && (i + 1 == a.subformulas_.end()))
        {
            strm << **i;
        }
        else
        {
            strm << (**i) << Operators[stoi(a.value_)];
        }
    }
    if (!no_parentheses)
    {
        strm << ")";
    }
    return strm;
}

Type StringToType(std::string op)
{
    if (op.size() > 1 || !isdigit(op[0]) || stoi(op) < 0 || stoi(op) > 6)
    {
        std::stringstream msg;
        msg << "StringToType Error: Unable to convert " << op
            << " to formula type, input must be an integer between 0 and 6.";
        throw std::runtime_error(msg.str().c_str());
    }
    int o = stoi(op);
    const Type TypeArray[] = {Type::FALSE, Type::TRUE, Type::NOT, Type::AND, Type::OR, Type::IMPLIES, Type::IFF};
    return TypeArray[o];
}

std::string TypeToString(Type op)
{
    const std::string TypeArray[] = {"FALSE", "TRUE", "NOT", "AND", "OR", "IMPLIES", "IFF"};
    return TypeArray[op];
}

size_t get_hash(const Formula &n)
{
    if (n.IsAtom() || !n.IsContingency())
    {
        return std::hash<std::string>()(n.GetName());
    }
    auto subformulas = n.GetSubformulas();
    if (subformulas.size() == 0)
    {
        return std::hash<std::string>()(n.GetName());
    }
    size_t s = std::hash<std::string>()(n.GetName());
    for (auto i = subformulas.begin(); i != subformulas.end(); ++i)
    {
        Formula fi = Formula(*i);
        s = hash_combine(s, get_hash(fi));
    }
    return s;
}
