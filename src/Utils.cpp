#include "Utils.h"

#include <algorithm>
#include <stdexcept>
#include "Formula.h"
#include "Kb.h"

Kb GetCardinalityConstraintNamed(CardinalityEncoding encoding, const std::vector<std::string> &atoms, int at_most,
                                 std::string id)
{
    if (encoding == 0)
    {
        return GetSequentialCounterEncoding(atoms, at_most, id);
    }
    else if (encoding == 1)
    {
        return GetTreeEncoding(atoms, at_most, id);
    }
    else if (encoding == 2)
    {
        return GetBinomialEncoding(atoms, at_most, id);
    }
    else
    {
        throw std::runtime_error("Unsupported cardinality encoding");
    }
}

Kb GetCardinalityConstraint(CardinalityEncoding encoding, const std::vector<std::string> &atoms, int at_most)
{
    if (encoding == 0)
    {
        return GetSequentialCounterEncoding(atoms, at_most, "Card");
    }
    else if (encoding == 1)
    {
        return GetTreeEncoding(atoms, at_most, "Card");
    }
    else if (encoding == 2)
    {
        return GetBinomialEncoding(atoms, at_most, "Card");
    }
    else
    {
        throw std::runtime_error("Unsupported cardinality encoding");
    }
}

// The sequential counter encoding, originally presented by Sinz [Towards an optimal CNF encoding of Boolean cardinality
// constraints. Springer, 2005]
Kb GetSequentialCounterEncoding(const std::vector<std::string> &atoms, int at_most, std::string id)
{
    Kb encoding = Kb();
    int n = atoms.size();

    std::string name = id + "_0_0";
    Formula c_00 = Formula(Type::OR, Formula(Type::NOT, atoms[0]), Formula(name));
    encoding.Add(c_00);

    // each formula has a register that counts true formulas in base one
    // i represents registers, j represents bits
    for (int j = 1; j <= at_most - 1; j++)
    {
        name = id + "_0_" + std::to_string(j);
        encoding.Add(Formula(Type::NOT, name));
    }
    for (int i = 1; i <= n - 2; i++)
    {
        name = id + "_" + std::to_string(i) + "_0";
        Formula c_i0 = Formula(Type::OR, Formula(Type::NOT, atoms[i]), Formula(name));
        encoding.Add(c_i0);

        name = id + "_" + std::to_string(i - 1) + "_0";
        Formula c_im0 = Formula(Type::OR, Formula(Type::NOT, name));
        name = id + "_" + std::to_string(i) + "_0";
        c_im0.AddSubformula(Formula(name));
        encoding.Add(c_im0);

        name = id + "_" + std::to_string(i - 1) + "_" + std::to_string(at_most - 1);
        Formula c_imnm = Formula(Type::OR, Formula(Type::NOT, atoms[i]), Formula(Type::NOT, name));
        encoding.Add(c_imnm);

        for (int j = 1; j <= at_most - 1; j++)
        {
            Formula c_r1 = Formula(Type::OR, Formula(Type::NOT, atoms[i]));
            std::string r_im_jm = id + "_" + std::to_string(i - 1) + "_" + std::to_string(j - 1);
            c_r1.AddSubformula(Formula(Type::NOT, r_im_jm));
            std::string r_ij = id + "_" + std::to_string(i) + "_" + std::to_string(j);
            c_r1.AddSubformula(Formula(r_ij));
            encoding.Add(c_r1);

            std::string r_im_j = id + "_" + std::to_string(i - 1) + "_" + std::to_string(j);
            Formula c_r2 = Formula(Type::OR, Formula(Type::NOT, r_im_j), Formula(r_ij));
            encoding.Add(c_r2);
        }
    }
    name = id + "_" + std::to_string(n - 2) + "_" + std::to_string(at_most - 1);
    Formula c_n = Formula(Type::OR, Formula(Type::NOT, atoms[n - 1]), Formula(Type::NOT, name));
    encoding.Add(c_n);
    return encoding;
}

// The tree-based encoding was originally presented by Bailleux and Boufkhad [Efficient CNF Encoding of Boolean
// Cardinality Constraints. Springer, 2013] This implementation uses the algorithm of Knuth [The Art of Computer
// Programming, Volume 4, Fascicle 6: Satisfiability. Addison Wesley, 2015] with improvements by Wynn [A comparison of
// encodings for cardinality constraints in a SAT solver, 2018]
Kb GetTreeEncoding(const std::vector<std::string> &atoms, int at_most, std::string id)
{
    int n = atoms.size();
    Kb encoding = Kb();

    // we use a vector to represent the nodes of a binary tree
    // internal nodes are in 1 to n-1
    // leaves are in indices n to 2*n-1
    std::vector<int> tree(2 * n);
    std::vector<int> leaves_below_node_k(2 * n);
    std::fill(tree.begin(), tree.end(), 1);
    std::fill(leaves_below_node_k.begin(), leaves_below_node_k.end(), 1);
    for (int k = n - 1; k >= 1; --k)
    {
        leaves_below_node_k[k] = leaves_below_node_k[2 * k] + leaves_below_node_k[2 * k + 1];
        // tree[k] represents the minimum of "at most" and the leaves below an internal node k, meaning
        // the maximum count of variables that we need to consider at a node k
        tree[k] = std::min(at_most, leaves_below_node_k[k]);
    }

    std::unordered_map<std::string, bool> required_atoms{};
    for (int k = 1; k <= n - 1; ++k)
    {
        for (int i = 1; i <= tree[2 * k]; ++i)
        {
            for (int j = 1; j <= tree[2 * k + 1]; ++j)
            {
                if ((i + j) == (at_most + 1))
                {
                    Formula clause = Formula(Type::OR);

                    int exponent = 2 * k;
                    if (i == 1 && n <= exponent && exponent < 2 * n)
                    {
                        // replace leaves with the original atoms
                        int atoms_index = (exponent - n + 1) - 1;
                        clause.AddSubformula(Formula(Type::NOT, atoms[atoms_index]));
                    }
                    else
                    {
                        std::string b = id + std::to_string(i) + "_" + std::to_string(exponent);
                        clause.AddSubformula(Formula(Type::NOT, b));
                        required_atoms.insert(std::make_pair(b, true));
                    }

                    exponent = 2 * k + 1;
                    if (j == 1 && n <= exponent && exponent < 2 * n)
                    {
                        // replace leaves with the original atoms
                        int atoms_index = (exponent - n + 1) - 1;
                        clause.AddSubformula(Formula(Type::NOT, atoms[atoms_index]));
                    }
                    else
                    {
                        std::string b = id + std::to_string(j) + "_" + std::to_string(exponent);
                        clause.AddSubformula(Formula(Type::NOT, b));
                        required_atoms.insert(std::make_pair(b, true));
                    }

                    encoding.Add(clause);
                }
            }
        }
    }

    for (int k = 2; k <= n - 1; ++k)
    {
        for (int m = 1; m <= tree[k]; ++m)
        {
            std::string b = id + std::to_string(m) + "_" + std::to_string(k);
            if (required_atoms.find(b) != required_atoms.end())
            {
                for (int i = 0; i <= tree[2 * k]; ++i)
                {
                    for (int j = 0; j <= tree[2 * k + 1]; ++j)
                    {
                        if ((i + j) == m)
                        {
                            Formula clause = Formula(Type::OR);
                            int exponent = 2 * k;
                            if (i == 1 && n <= exponent && exponent < 2 * n)
                            {
                                // replace leaves with the original atoms
                                int atoms_index = (exponent - n + 1) - 1;
                                clause.AddSubformula(Formula(Type::NOT, atoms[atoms_index]));
                            }
                            else if (i != 0)
                            {
                                std::string b = id + std::to_string(i) + "_" + std::to_string(exponent);
                                clause.AddSubformula(Formula(Type::NOT, b));
                                required_atoms.insert(std::make_pair(b, true));
                            }

                            exponent = 2 * k + 1;
                            if (j == 1 && n <= exponent && exponent < 2 * n)
                            {
                                // replace leaves with the original atoms
                                int atoms_index = (exponent - n + 1) - 1;
                                clause.AddSubformula(Formula(Type::NOT, atoms[atoms_index]));
                            }
                            else if (j != 0)
                            {
                                std::string b = id + std::to_string(j) + "_" + std::to_string(exponent);
                                clause.AddSubformula(Formula(Type::NOT, b));
                                required_atoms.insert(std::make_pair(b, true));
                            }

                            std::string b_m_k = id + std::to_string(m) + "_" + std::to_string(k);
                            clause.AddSubformula(Formula(b_m_k));
                            encoding.Add(clause);
                        }
                    }
                }
            }
        }
    }
    return encoding;
}

// Binomial (brute-force) encoding
// To represent a1 + ... + an <=k, we add all clauses that are disjunctions of
// subsets of size k+1 of {!a1,...,!an}
Kb GetBinomialEncoding(const std::vector<std::string> &atoms, int at_most, std::string id)
{
    auto subsets = getSubsetsOfSizeK(atoms, at_most + 1);
    Formula cardinality_constraint = Formula(Type::AND);
    for (auto subset : subsets)
    {
        Formula disj = Formula(Type::OR);
        for (auto atom : subset)
        {
            disj.AddSubformula(Formula(Type::NOT, atom));
        }
        cardinality_constraint.AddSubformula(disj);
    }
    return Kb(cardinality_constraint);
}

std::vector<std::vector<std::string>> getSubsetsOfSizeK(const std::vector<std::string> &atoms, int at_most)
{
    std::vector<std::string> subsets{};
    std::vector<std::vector<std::string>> result;
    getSubsetsRecursive(atoms, at_most, 0, subsets, result);
    return result;
}

void getSubsetsRecursive(const std::vector<std::string> &atoms, int k, int index, std::vector<std::string> &sub,
                         std::vector<std::vector<std::string>> &result)
{
    if (k == 0)
    {
        result.push_back(sub);
    }
    for (int i = index; i < atoms.size(); ++i)
    {
        sub.push_back(atoms[i]);
        getSubsetsRecursive(atoms, k - 1, i + 1, sub, result);
        sub.pop_back();
    }
}