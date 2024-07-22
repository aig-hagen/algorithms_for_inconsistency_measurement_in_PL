#include <sstream>
#include <set>
#include <vector>
#include <iostream>

#include "ProblematicIterativeASP.h"
#include "MvIterativeASP.h"
#include "InconsistencyMeasureASP.h"
#include "Parser.h"
#include "Constants.h"
#include <clingo.hh>

double p_measure_MUS_based(Kb& kb);
double mv_measure_MUS_based(Kb& kb);
