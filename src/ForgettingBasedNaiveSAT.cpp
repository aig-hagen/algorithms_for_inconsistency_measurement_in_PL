// Naive Implementation of the Forgetting Based Inconsistency Measure
#include <iostream>
#include <set>
#include <iterator>
#include <sstream>
#include <vector>
#include <string>
#include <tuple>

#include "ForgettingBasedNaiveSAT.h"
#include "Utils.h"
#include "SatSolver.h"
#include "Formula.h"
#include "Kb.h"
#include "Parser.h"
// checks if there are no multiple substitutions for the same atom occurence occurrence in the given set of substitutions
// e.g. {("a1", 7, true),("a1", 7, false)} would not be a valid set of substitutions and the value false would be returned  
bool are_substitutions_valid(std::set<std::tuple<std::string, int, bool>> substitutions){
	for(std::tuple<std::string, int, bool> elem1: substitutions){
		for(std::tuple<std::string, int, bool> elem2: substitutions){
			if(elem1 != elem2 && std::get<0>(elem1) == std::get<0>(elem2) && std::get<1>(elem1) == std::get<1>(elem2)){
				return false;
			}
		}
	}
	return true;
}

// function that takes a set of substitutions and performs them on the given formula f
// returns a copy of f, where the substitutions have been performed
Formula perform_substitutions(Formula f, std::set<std::tuple<std::string, int, bool>> substitutions){
	// Verum (top) and Falsum (bot) constants
	Formula top = Formula(Type::TRUE);
	Formula bot = Formula(Type::FALSE);
	for(std::tuple<std::string, int, bool> substitution: substitutions){
		if(std::get<2>(substitution))
			f.ReplaceIthOccurenceOfAtom(std::get<0>(substitution), top, std::get<1>(substitution));
		else
			f.ReplaceIthOccurenceOfAtom(std::get<0>(substitution), bot, std::get<1>(substitution));
	}
	//std::cout << f << std::endl;
	return f;
}
int forgetting_based_naive_measure(Kb& k)
{
   // initialize SAT-solver
   SatSolver solver = SatSolver();
   
   // Put every forula in the given kb into a single conjunction
   Formula kb_as_conjunction = Formula(Type::AND); // empty conjunction
   for (Formula f : k.GetFormulas()){
	   kb_as_conjunction.AddSubformula(f);
   }
   // Generate set of all possible Substitutions, substitutions are triples of the form(<atom_name>, <occurrence>, <true or false>),
   // e.g. ("a2", 4, true) means that the 4th occurrence of atom a2 is replaced by the constant Verum
   std::set<std::tuple<std::string, int, bool>> substitutions;
   std::set<std::string> sig = kb_as_conjunction.GetSignature();
   for(std::string atom : sig){
	   int n = kb_as_conjunction.GetNumberOfAtomOccurences(atom);
	   for(int i = 1; i <= n; i++){
		   // add the 2 substitutions to the set of all substitutions
		   std::tuple<std::string, int, bool> s_top = std::make_tuple(atom, i, true);
		   std::tuple<std::string, int, bool> s_bot = std::make_tuple(atom, i, false); 		   
		   substitutions.insert(s_top);
		   substitutions.insert(s_bot);
	   }
   }
   
   /* figuring out how tuples work in cpp
   std::tuple<std::string, int, bool> testtuple= std::make_tuple("atom", 2, true);
   std::cout << "First val " << std::get<0>(testtuple) << std::endl; 
   std::cout << "Snd val " << std::get<1>(testtuple) << std::endl; 
   std::cout << "Thd val " << std::get<2>(testtuple) << std::endl; */
   // testing subst set
   
   /* testing set of all substitutions, (seems to work)
   std::set<std::tuple<std::string, int, bool>>::iterator itr;
   for(itr = substitutions.begin(); itr != substitutions.end(); itr++){
	   std::cout << "(" << std::get<0>(*itr) << ", " << std::get<1>(*itr) << ", " << std::get<2>(*itr) << ")" << std::endl;
   }
   */
   
   // Cycle through the subsets of the set of all substitutions, first all sets of size 1, then all of size 2 etc.
   // Hard-coded For subset size 2
   // initializing pointers vectore
   
   
   
   //check if kb is consistent or not
   if (solver.IsSatisfiable(kb_as_conjunction, false)){
				return 0;
	}
   
   for(int i = 1; i <= kb_as_conjunction.GetNumberOfAtomOccurences(); i++){
   //std::cout << "Initializing array of iterators of size i=" << i << std::endl;
   //initialize vector of iterators
   std::vector<std::set<std::tuple<std::string, int, bool>>::iterator> iterators = {};
   for(int j = 0; j < i; j++){
	   iterators.push_back(substitutions.begin());
	   std::advance(iterators.at(j), j);
   }
   //std::cout << "DONE Initializing array of iterators of size i=" << i << std::endl;
   //std::cout << "Array of pointers has size " << iterators.size() << std::endl;
   //Go through all subsets of size i of the set of all substitutions (i.e. substitutions)
   while(iterators.back() != substitutions.end()){
		//std::cout << "Entered while loop i=" << i << std::endl;
		// build subset of substitutions that needs to be checked
		std::set<std::tuple<std::string, int, bool>> current_combination;
		for(int k = 0; k < iterators.size(); k++){
			current_combination.insert(*iterators.at(k));  
		}
		//std::cout << "LOADED current permutation" << i << std::endl;
		if(are_substitutions_valid(current_combination)){
			// perform the substistutions
			//std::cout << "Performing substitutions" << i << std::endl;
			Formula f = perform_substitutions(kb_as_conjunction, current_combination);
			//std::cout << "DONE Performing substitutions" << i << std::endl;
			// call sat solver to check if the formula is now satisfiable or not
			//std::cout << "Calling SAT-Solver to see if satisfiable after substituting" << i << std::endl;
			if (solver.IsSatisfiable(f, false)){
				//std::cout << "SATISFIABLE" << i << std::endl;
				return i;
			}
			//std::cout << "UNSATISFIABLE" << i << std::endl;
		}
		// update iterators for next iteration i.e the next subset of substitutions
		//std::cout << "Updating Vector of Iterators" << i << std::endl;
		iterators.back()++; // start by increasing the rightmost iterator by one
		
		int l = iterators.size() - 2; // l is the index of the second iterator from the right
		// if the rightmost iterator has gone out of bounds then the iterators left of it need to be moved
		while(iterators.back() == substitutions.end() && l >= 0){
		// then the closest iterator to the left needs to be increased by one
			iterators.at(l)++;
			// every iterator right of index l needs to be reset correctly
			for(int j = l+1; j < iterators.size(); j++){
				iterators.at(j) = iterators.at(l);
				std::advance(iterators.at(j), j-l);
			}
			l--;
		}
   }
   }
   
   //Testing iterators 
   /*
   std::set<std::tuple<std::string, int, bool>> testset;
   testset.insert(std::make_tuple("a", 1, true));
   testset.insert(std::make_tuple("b", 2, true));
   testset.insert(std::make_tuple("c", 3, true));
   std::set<std::tuple<std::string, int, bool>>::iterator itr2;
   itr2 = testset.begin();
   std::cout << "Iterator at testset.begin offset 0" << std::get<0>(*itr2) << std::endl;
   //std::advance(itr2, 2);
   
   std::vector<std::set<std::tuple<std::string, int, bool>>::iterator> pointers1 = {};
   pointers1.push_back(itr2);
   pointers1.push_back(testset.begin());
   pointers1.push_back(testset.begin());
   pointers1.at(1)++;
   std::advance(pointers1.at(2), 2);
   std::cout << "Vector Length " << pointers1.size() << std::endl;
   std::cout << "Iterator at testset.begin offset 0" << std::get<0>(*(pointers1.at(0))) << std::endl;
   std::cout << "Iterator at testset.begin offset 1" << std::get<0>(*(pointers1.at(1))) << std::endl;
   std::cout << "Iterator at testset.begin offset 2" << std::get<0>(*(pointers1.at(2))) << std::endl;
   std::advance(pointers1.at(1), 2);
   if(pointers1.at(1) != testset.end()){
   std::cout << "Is pointer at 1 unequal testset.end()? " << "yes" << std::endl;
   }
   else{
	  std::cout << "Is pointer at 1 unequal testset.end()? " << "no" << std::endl;
   }
   if(pointers1.at(2) != testset.end()){
   std::cout << "Is pointer at 2 unequal testset.end()? " << "yes" << std::endl;
   }
   else{
	  std::cout << "Is pointer at 2 unequal testset.end()? " << "no" << std::endl;
   }
   
   */
   //std::cout << "Iterator at testset.begin offset 2" << std::get<0>(*(itr2+2)) << std::endl;
   // test and understand this 
   /*void iterate(int *a, int i, int size, int n){
	int start = 0;
	if( i > 0 ) start = a[i-1]+1;
	for(a[i] = start; a[i] < n; a[i]++) {
	if(i == n-1) {
      // a is the array of indices of size n
      for( int k = 0; k < size; k++ ) {
          printf("%d ",a[k]);
      }
      printf("\n");
	}
        else
            iterate(a, i+1, size, n);
    }
	}*/
   /* See if helper function are_substitutions_valid works as intended (seems to work fine)
   std::set<std::tuple<std::string, int, bool>> invalid;
   invalid.insert(std::make_tuple("a", 2, true));
   invalid.insert(std::make_tuple("b", 2, true));
   invalid.insert(std::make_tuple("a", 2, false));
   invalid.insert(std::make_tuple("c", 4, true));
   
   std::set<std::tuple<std::string, int, bool>> valid;
   valid.insert(std::make_tuple("a", 2, true));
   valid.insert(std::make_tuple("b", 2, true));
   valid.insert(std::make_tuple("a", 3, true));
   valid.insert(std::make_tuple("c", 4, true));
   
   std::cout << are_substitutions_valid(invalid) << std::endl;
   std::cout << are_substitutions_valid(valid) << std::endl;
   */
   
   /* Testing helper function perform_substitutions (seems to work fine)
   std::set<std::tuple<std::string, int, bool>> valid2;
   valid2.insert(std::make_tuple("A1", 1, true));
   valid2.insert(std::make_tuple("A0", 2, false));
   valid2.insert(std::make_tuple("A2", 2, false));
   valid2.insert(std::make_tuple("c", 4, true));
   
   std::cout << "pre substitution " << kb_as_conjunction << std::endl;
   Formula substituted_f = perform_substitutions(kb_as_conjunction, valid2);
   std::cout << "formula with substitutions" << substituted_f << std::endl;
   /*
   
   std::cout << "post substitution " << kb_as_conjunction << std::endl;
   /* Useful functions
    void ReplaceIthOccurenceOfAtom(std::string atom, const Formula &f, int i);
	kb_as_conjunction.GetNumberOfAtomOccurences(atom);
   */
   
	return 512;
}