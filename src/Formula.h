#ifndef PLFORMULA_H
#define PLFORMULA_H

#include <memory>
#include <set>
#include <string>
#include <vector>

// The different operators used by propositional formulas.
enum Type
{
    // Name | int | Tweety equivalent
    FALSE,   // 0    +
    TRUE,    // 1    -
    NOT,     // 2    !
    AND,     // 3    &&
    OR,      // 4    ||
    IMPLIES, // 5    =>
    IFF      // 6    <=>
};

// This class represents a formula in tree form.
// Tree nodes are non-atomary formulas. Leaves represent atoms.
// e.g:
// The formula "!(a && b && !c)" is represented by
//         NOT
//          |
//         AND
//   |      |     |
//   a      b     NOT
//                 |
//                 c
class Formula
{
  private:
    // For tree nodes (non-atomary formulas), this value represents the operator
    // of the formula (see enum 'Type' for available types. For leaves (atoms),
    // this value is the name of the atom.
    std::string value_;

    // The maximum number of subformulas depends on the type of formulas.
    // Negations always have exactly one subformula. Conjunctions and disjunctions
    // can have any number of subformulas. Implications and equivalences
    // have two subformulas.
    std::vector<std::shared_ptr<Formula>> subformulas_;

    // Simplify nested associative formulas
    // i.e. a&&(b&&c) becomes (a&&b&&c)
    void TrimAssociativeFormula();

    Formula ReplaceIthOccurenceOfAtomRecursive(std::string atom, const Formula &f, int i, int &j) const;
    void GetPermutations(std::vector<std::vector<Formula>> &in, size_t i, std::vector<Formula> &accum,
                         std::vector<std::vector<Formula>> &out) const;

  public:
    // Copy-constructor
    Formula(const Formula &f);
    // Create a new atom.
    Formula(std::string atom);
    // Create a new (empty) formula with the given operator
    // or a tautology/contradiction if the operator is "TRUE"/"FALSE"
    Formula(Type op);
    // Create a new formula with the given operator and subformula.
    Formula(Type op, const Formula &f);
    // Create a new formula with the given operator and subformulas.
    Formula(Type op, const Formula &a, const Formula &b);
    // Create a new formula with the given operator and list of subformulas.
    Formula(Type op, const std::vector<Formula> &formulas);

    // Add another conjunct or disjunct to an existing associative
    // formula of the same type,
    // or add another subformula to an incomplete formula of another type.
    void AddSubformula(const Formula &f);

    inline std::string GetName() const
    {
        return this->value_;
    }

    std::vector<std::shared_ptr<Formula>> GetSubformulasPtrs() const
    {
        return this->subformulas_;
    }

    std::vector<Formula> GetSubformulas() const;

    // Get the number of subformulas
    inline int size() const
    {
        return this->subformulas_.size();
    }

    inline bool empty() const
    {
        return (this->subformulas_.size()) == 0;
    }

    // Get the number of (non-unique) atom occurences in the formula.
    int GetNumberOfAtomOccurences() const;
    // Get the number of occurences of the specified atom in the formula.
    int GetNumberOfAtomOccurences(std::string atom) const;
    // Get the set of unique atom names in this formula.
    std::set<std::string> GetSignature() const;

    // Functions for checking the type of a formula.
    inline bool IsAtom() const
    {
        return !isdigit(this->value_[0]);
    }
    inline bool IsLiteral() const
    {
        if (this->IsAtom())
        {
            return true;
        }
        if (this->IsNegation() && this->size() == 1)
        {
            return (this->subformulas_[0])->IsAtom();
        }
        else
        {
            return false;
        }
    }
    inline bool IsContradiction() const
    {
        return !IsAtom() && (stoi(this->value_) == 0);
    }
    inline bool IsTautology() const
    {
        return !IsAtom() && (stoi(this->value_) == 1);
    }
    inline bool IsContingency() const
    {
        return !IsTautology() && !IsContradiction();
    }
    inline bool IsConjunction() const
    {
        return !IsAtom() && (stoi(this->value_) == 3);
    }
    inline bool IsDisjunction() const
    {
        return !IsAtom() && (stoi(this->value_) == 4);
    }
    // Note: This only checks for and/or, not equivalence
    inline bool IsAssociativeFormula() const
    {
        return IsConjunction() || IsDisjunction();
    }
    inline bool IsNegation() const
    {
        return !IsAtom() && (stoi(this->value_) == 2);
    }
    inline bool IsImplication() const
    {
        return !IsAtom() && (stoi(this->value_) == 5);
    }
    inline bool IsEquivalence() const
    {
        return !IsAtom() && (stoi(this->value_) == 6);
    }
	// Types of formulas that are ltl specific
	inline bool IsNext() const
    {
        return !IsAtom() && (stoi(this->value_) == 7);
    }
    inline bool IsUntil() const
    {
        return !IsAtom() && (stoi(this->value_) == 8);
    }
    inline bool IsGlobally() const
    {
        return !IsAtom() && (stoi(this->value_) == 9);
    }
    inline bool IsFinally() const
    {
        return !IsAtom() && (stoi(this->value_) == 10);
    }

    // Replace the ith occurence of the atom with the given name with the given
    // formula.
    void ReplaceIthOccurenceOfAtom(std::string atom, const Formula &f, int i);
    // Replace the alles occurences of the atom with the given name with the given
    // formula.
    void ReplaceAllOccurencesOfAtom(std::string atom, const Formula &f);

    // Replace the vector of subformulas entirely
    void ReplaceSubformulas(std::vector<Formula> new_subformulas);

    // Convert the formula to CNF using the naive method.
    Formula NaiveToCnf() const;

    // Overloaded operators
    friend std::ostream &operator<<(std::ostream &strm, const Formula &a);
    friend bool operator==(const Formula &a, const Formula &b);
    friend bool operator!=(const Formula &a, const Formula &b);
};

// Utility function for converting a string digit back to a Type
// only works if the input string is in {"0","1","2","3","4","5","6"})
Type StringToType(std::string op);

// Utility function for getting a Type in string format (i.e. "NOT" instead of
// "2")
std::string TypeToString(Type op);

template <typename T> size_t hash_combine(std::size_t s, const T &n)
{
    s ^= std::hash<T>()(n) + 0x9e3779b9 + (s << 6) + (s >> 2);
    return s;
}

size_t get_hash(const Formula &n);

// Custom hash function for Formula
namespace std
{
template <> struct hash<Formula>
{
    std::size_t operator()(const Formula &n) const
    {
        return get_hash(n);
    }
};
} // namespace std

#endif /* FORMULA_H */
