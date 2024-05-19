#include <iostream>
#include "ForgettingBasedNaiveSAT.h"
double get_inconsistency_naive_value(Kb& k, ImSettings config)
{	
	// there is only the naive implementation for the forgetting based measure for now 
    return forgetting_based_naive_measure(k);
}