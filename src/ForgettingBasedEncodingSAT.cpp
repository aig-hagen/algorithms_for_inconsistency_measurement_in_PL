#include <iostream>
#include <set>
#include <sstream>
#include <vector>
#include <string>

#include "InconsistencyMeasureSAT.h"
#include "Utils.h"
#include "Formula.h"
#include "Kb.h"
#include "Parser.h"

Kb GetForgettingEncoding(const Kb &k, int u, CardinalityEncoding enc)
{
    //Maybe add tseitsin encoding?
    //Step 0: Check for trivial cases:
    if (u == 0) // no atoms are allowed to be forgotten, encoding is unnecessary
    {
        return Kb(k);
    }
    Kb encoding = Kb(); // the SAT-encoding for the given knowledge base k, for the given parameter u
    if (k.GetFormulas().empty()) // input kb is empty and therefore trivially consistent, return empty encoding
    {
        return encoding;
    }

    // Step 1: Introduce all needed new atoms
    // for each atom occurence ai in k, introduce 2 new atoms (ti, fi)
    int Num_of_Occs_in_k = k.GetNumberOfAtomOccurences();
    std::vector<std::string> new_atoms;
    for (int i = 1; i <= k.GetNumberOfAtomOccurences() ; i++){
        new_atoms.push_back("FORGET_T" + std::to_string(i)); // the new variables are actually called FORGET_T and FORGET_Y here, because
        new_atoms.push_back("FORGET_F" + std::to_string(i)); // we need to avoid any overlap of the names of the new atoms with the
                                                             // original atoms in k
    }

    // Step 2: Add rules that simulate the ability to forget single atom occurences in k
    // For each atom occurence ai in given knowledge base k, replace the occurence with
    // the subformula (ti || ai) && !fi
    int counter = 1;
    //GetFormulas constructs a copy of the formulas contained in the kb
    //(so you're not working on the same objects)
    for (Formula f : k.GetFormulas()){
        std::set<std::string> sig = f.GetSignature();
        for (std::string atom : sig){
            int numOccs = f.GetNumberOfAtomOccurences(atom);
            for (int i = 1; i <= numOccs; i++){
                // constructing the substitute (ti || ai) && !fi
                Formula ai = Formula(atom);
                Formula ti = Formula("FORGET_T" + std::to_string(counter));
                Formula not_fi = Formula(Type::NOT, Formula("FORGET_F" + std::to_string(counter)));
                Formula substitute = Formula(Type::AND, Formula(Type::OR, ti, ai), not_fi);
                // replace
                f.ReplaceIthOccurenceOfAtom(atom, substitute, i);
                counter++;
            }
        }
        // once all atom occurences in the formula have been replaced, add modified formula to encoding
        encoding.Add(f);
    }
    // Step 3: Add constraints ensuring that not both new atoms of any pair (ti,fi) are assigned true,
    // as this is not a valid state in the chosen encoding
    for (int i = 1; i <= Num_of_Occs_in_k; i++){
        Formula not_ti = Formula(Type::NOT, Formula("FORGET_T" + std::to_string(i)));
        Formula not_fi = Formula(Type::NOT, Formula("FORGET_F" + std::to_string(i)));
        Formula f = Formula(Type::OR, not_ti, not_fi);
        encoding.Add(f);
    }
    // Step 4: Finally, limit the number of atom occurences that are allowed to be forgotten to u
    encoding.Add(GetCardinalityConstraint(enc, new_atoms, u));

    // Return the completed encoding
    return encoding;
}
