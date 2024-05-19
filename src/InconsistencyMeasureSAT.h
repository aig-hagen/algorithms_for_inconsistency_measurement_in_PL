#ifndef INCONSISTENCYMEASURE_H
#define INCONSISTENCYMEASURE_H

#include "Kb.h"
#include "Utils.h"
#include <functional>
#include <string>

// Basic sat-based algorithm for inconsistency measures
//"infinity" results are returned as "-1" (parsed as "INF" in main.cpp)
double BinarySearchInconsistencyValue(const Kb &k, int max,
                                      std::function<Kb(const Kb &k, int u, CardinalityEncoding enc)> GetSatEncoding,
                                      const ImSettings &config);

// Linear search alternative for sat-based implementations of inconsistency measures
double LinearSearchInconsistencyValue(const Kb &k, int max,
                                      std::function<Kb(const Kb &k, int u, CardinalityEncoding enc)> GetSatEncoding,
                                      const ImSettings &config);

// Get the inconsistency value for the given knowledge base by calling
// BinarySearchInconsistencyValue with the corresponding GetSatEncoding
// function with the corresponding range.
// The parameter "config" defines the inconsistency measure, the cardinality
// encoding, and other options (see struct definition above).
double GetInconsistencyValue(const Kb &k, ImSettings &config);

// Alternative to GetInconsistencyValue
// calls LinearSearchInconsistencyValue instead of BinarySearchInconsistencyValue
double GetInconsistencyValueLinSearch(const Kb &k, ImSettings &config);

// The following functions return SAT encodings of the
// problem upper_I for the given
// knowledge bases k and upper limits u
Kb GetHsEncoding(const Kb &k, int u, CardinalityEncoding enc);
Kb GetContensionEncoding(const Kb &k, int u, CardinalityEncoding enc); // implemented in separate source file ContensionEncoding.cpp
Kb GetForgettingEncoding(const Kb &k, int u, CardinalityEncoding enc); // implemented in separate source file ForgettingEncoding.cpp
//Kb GetCommonDistanceEncoding(const Kb &k);
Kb GetMaxDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc);
Kb GetHitDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc);
Kb GetSumDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc);


#endif /* INCONSISTENCYMEASURE_H */
