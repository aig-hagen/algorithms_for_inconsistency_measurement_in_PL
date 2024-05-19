#ifndef INCONSISTENCYMEASURE_MAXSAT_H
#define INCONSISTENCYMEASURE_MAXSAT_H

#include "Kb.h"
#include "Utils.h"
#include <functional>
#include <string>
//#include <EvalMaxSAT.h>
// #include "../lib/EvalMaxSAT/lib/EvalMaxSAT/src/EvalMaxSAT.h"

//double get_inconsistency_value_MaxSAT(const Kb &k, ImSettings &config);

//VL: Is this function obsolete? It doesn't seem to be used anywhere.
double SearchInconsistencyValueMaxSAT(const Kb &k, 
                                std::function<Kb(const Kb &k)> GetContensionEncodingMaxSATHardClauses,
                                std::function<Kb(const Kb &k)> GetContensionEncodingMaxSATSoftClauses);

// 
// double GetInconsistencyValueMaxSAT(const Kb &k, ImSettings &config);

// The following functions return MaxSAT encodings 
Kb GetContensionEncodingMaxSATHardClauses(const Kb &k);
Kb GetContensionEncodingMaxSATSoftClauses(const Kb &k);

// Kb GetHsEncoding(const Kb &k, int u, CardinalityEncoding enc);
// Kb GetContensionEncodingMaxSAT(const Kb &k, int u, CardinalityEncoding enc); // implemented in separate source file ContensionEncodingMaxSAT.cpp
// Kb GetCommonDistanceEncoding(const Kb &k);
// Kb GetMaxDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc);
// Kb GetHitDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc);
// Kb GetSumDistanceEncoding(const Kb &k, int u, CardinalityEncoding enc);
// Kb GetForgettingEncoding(const Kb &k, int u, CardinalityEncoding enc); // implemented in separate source file ForgettingEncoding.cpp
/* INCONSISTENCYMEASURE_MAXSAT_H */

#endif 
