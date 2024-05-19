#ifndef UTILS_H
#define UTILS_H

#include "Formula.h"
#include "Kb.h"
#include <string>
#include <unordered_map>
#include <vector>

// Available cardinality encodings
enum CardinalityEncoding
{
    SEQUENTIAL_COUNTER,
    TREE,
    BINOMIAL
};

// This structs collects configuration settings
// for a specific GetInconsistencyValue call.
struct ImSettings
{
    std::string measure_name;                 // name of the measure
    CardinalityEncoding cardinality_encoding; // the cardinality encoding that is
                                              // used, if applicable
    bool is_in_cnf;                           // if set to true, the sat solver does not convert the input
                                              // encoding into CNF
    int offset;                               // a value that is added to the inconsistency value
	int m;                          		  // the number of the final state (only relevant for contension-ltl and drastic-ltl)
};

// Generates a cardinality constraint of the form
//"<at_most> out of <atoms> are allowed to be true"
// The parameter <id> is used to name the cardinality atoms.
// Use this method (with a unique <id>) instead of GetCardinalityConstraint
// if you need more than one cardinality constraint, otherwise
// the signatures of the constraints will overlap.
Kb GetCardinalityConstraintNamed(CardinalityEncoding encoding, const std::vector<std::string> &atoms, int at_most,
                                 std::string id);

// Generates a cardinality constraint of the form
//"<at_most> out of <atoms> are allowed to be true"
Kb GetCardinalityConstraint(CardinalityEncoding encoding, const std::vector<std::string> &atoms, int at_most);

Kb GetSequentialCounterEncoding(const std::vector<std::string> &atoms, int at_most, std::string id);
Kb GetTreeEncoding(const std::vector<std::string> &atoms, int at_most, std::string id);
Kb GetBinomialEncoding(const std::vector<std::string> &atoms, int at_most, std::string id);

void getSubsetsRecursive(const std::vector<std::string> &atoms, int k, int index, std::vector<std::string> &sub,
                         std::vector<std::vector<std::string>> &result);
std::vector<std::vector<std::string>> getSubsetsOfSizeK(const std::vector<std::string> &atoms, int at_most);

// Helper function for getting the current max second value of map
template <typename T> inline int GetMaxEntryInMap(std::unordered_map<T, int> &m)
{
    int max = 1;
    typename std::unordered_map<T, int>::iterator entry;
    for (entry = m.begin(); entry != m.end(); ++entry)
    {
        if (entry->second > max)
        {
            max = entry->second;
        }
    }
    return max;
}

namespace time_msrs{
    extern int num_solver_calls; // Number of times the solver is called to find the inconsistency value
    extern std::vector<double> encoding_times; // times needed to generate SAT-Encoding/ASP-Rules
    extern std::vector<double> cnf_transform_times; // times needed to transform SAT-Encoding into CNF
    extern std::vector<double> encoding_sizes; // sizes in bytes of the sat encoding for each iteration
    extern std::vector<double> solver_times_pure; // Times needed to solve SAT-Encoding
                                                  // (without counting cadical preprocessing)
    extern std::vector<double> solver_times_total; // Times needed for the complete solving process of a
                                                   // SAT-Encoding/the ASP-Program including solver preprocessing
}

#endif /* UTILS_H */
