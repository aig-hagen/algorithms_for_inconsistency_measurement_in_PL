#include <chrono>

#include "InconsistencyMeasureSAT.h"
#include "SatSolver.h"
#include "Utils.h"

#include <iostream>

// double BinarySearchInconsistencyValue(const Kb &k, int max,
//                                       std::function<Kb(const Kb &k, int u, CardinalityEncoding enc)> GetSatEncoding,
//                                       const ImSettings &config)
// {
//     int offset = config.offset;
//     bool is_in_cnf = config.is_in_cnf;
//     CardinalityEncoding enc = config.cardinality_encoding;

//     int min = 0 + offset;
//     SatSolver solver = SatSolver();
//     while (min <= max)
//     {
//         int mid = min + (max - min) / 2;
//         // measure the time it takes to generate the SAT-Encoding
//         auto encoding_start = std::chrono::steady_clock::now();
//         Kb encoding = GetSatEncoding(k, mid, enc);
//         auto encoding_end = std::chrono::steady_clock::now();
//         double interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
//                           / double (1000000.0);
//         time_msrs::encoding_times.push_back(interval);

//         if (solver.IsSatisfiable(encoding, is_in_cnf))
//         {
//             if (mid == (0 + offset))
//             {
//                 return 0.0;
//             }
//             Kb encoding_lower = GetSatEncoding(k, mid - 1, enc);
//             if (!solver.IsSatisfiable(encoding_lower, is_in_cnf))
//             {
//                 return (double)(mid - offset);
//             }
//             else
//             {
//                 max = mid;
//             }
//         }
//         else
//         {
//             min = mid + 1;
//         }
//     }
//     return -1; // infinity case or error (TODO: use config to differentiate errors and infinity cases)
// }

double BinarySearchInconsistencyValue(const Kb &k, int max,
                                      std::function<Kb(const Kb &k, int u, CardinalityEncoding enc)> GetSatEncoding,
                                      const ImSettings &config)
{
    int offset = config.offset;
    bool is_in_cnf = config.is_in_cnf;
    CardinalityEncoding enc = config.cardinality_encoding;

    int inc_val = -1;

    int min = 0 + offset; 
    SatSolver solver = SatSolver();
    while (min <= max)
    {
        int mid = min + (max - min) / 2;
        // measure the time it takes to generate the SAT-Encoding
        auto encoding_start = std::chrono::steady_clock::now();
        Kb encoding = GetSatEncoding(k, mid, enc);
        auto encoding_end = std::chrono::steady_clock::now();
        double interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                          / double (1000000.0);
        time_msrs::encoding_times.push_back(interval);

        if (solver.IsSatisfiable(encoding, is_in_cnf))
        {
            if (inc_val < 0 || (mid - offset) < inc_val)
                inc_val = mid - offset;
            max = mid - 1; 
        }
        else
        {
            min = mid + 1;
        }
    }

    return inc_val;
}

double LinearSearchInconsistencyValue(const Kb &k, int max,
                                      std::function<Kb(const Kb &k, int u, CardinalityEncoding enc)> GetSatEncoding,
                                      const ImSettings &config)
{
    bool is_in_cnf = config.is_in_cnf;
    CardinalityEncoding enc = config.cardinality_encoding;

    int inc_val = 0;

    SatSolver solver = SatSolver();
    while (inc_val <= max)
    {

        // measure the time it takes to generate the SAT-Encoding
        auto encoding_start = std::chrono::steady_clock::now();
        Kb encoding = GetSatEncoding(k, inc_val, enc);
        auto encoding_end = std::chrono::steady_clock::now();
        double interval = double (std::chrono::duration_cast<std::chrono::microseconds>(encoding_end - encoding_start).count())
                          / double (1000000.0);
        time_msrs::encoding_times.push_back(interval);

        if (solver.IsSatisfiable(encoding, is_in_cnf))
        {
            return inc_val;
        }
        else
        {
            inc_val++;
        }
    }
    // This should not happen, the insonsistency value cannot be higher than max
    return -1;
}

double GetInconsistencyValue(const Kb &k, ImSettings &config)
{
    if (config.measure_name == "contension")
    {
        if (k.size() == 0)
        {
            return 0;
        }
        int max = (k.GetSignature()).size();
        config.is_in_cnf = true;
        return BinarySearchInconsistencyValue(k, max, GetContensionEncoding, config);
    }
    else if (config.measure_name == "forget")
    {
        int max = k.GetNumberOfAtomOccurences();
        return BinarySearchInconsistencyValue(k, max, GetForgettingEncoding, config);
    }
    else if (config.measure_name == "hs")
    {
        if (k.size() == 0)
        {
            return 0;
        }
        int max = k.size();
        config.offset = 1;
        return BinarySearchInconsistencyValue(k, max, GetHsEncoding, config);
    }
    else if (config.measure_name == "hitdalal")
    {
        int max = k.size();
        int value = BinarySearchInconsistencyValue(k, max, GetHitDistanceEncoding, config);
        if (value == -1)
        {
            return max;
        }
        else
        {
            return value;
        }
    }
    else if (config.measure_name == "maxdalal")
    {
        int max = (k.GetSignature()).size();
        return BinarySearchInconsistencyValue(k, max, GetMaxDistanceEncoding, config);
    }
    else if (config.measure_name == "sumdalal")
    {
        int max = k.size() * (k.GetSignature()).size();
        return BinarySearchInconsistencyValue(k, max, GetSumDistanceEncoding, config);
    }
    else
    {
        throw std::runtime_error("Unknown inconsistency measure");
    }
}

double GetInconsistencyValueLinSearch(const Kb &k, ImSettings &config)
{
    if (config.measure_name == "contension")
    {
        if (k.size() == 0)
        {
            return 0;
        }
        int max = (k.GetSignature()).size();
        config.is_in_cnf = true;
        return LinearSearchInconsistencyValue(k, max, GetContensionEncoding, config);
    }
    else if (config.measure_name == "forget")
    {
        int max = k.GetNumberOfAtomOccurences();
        return LinearSearchInconsistencyValue(k, max, GetForgettingEncoding, config);
    }
    /*else if (config.measure_name == "hs")
    {
        if (k.size() == 0)
        {
            return 0;
        }
        int max = k.size();
        config.offset = 1;
        return LinearSearchInconsistencyValue(k, max, GetHsEncoding, config);
    }*/
    else if (config.measure_name == "hitdalal")
    {
        int max = k.size();
        int value = LinearSearchInconsistencyValue(k, max, GetHitDistanceEncoding, config);
        if (value == -1)
        {
            return max;
        }
        else
        {
            return value;
        }
    }
    else if (config.measure_name == "maxdalal")
    {
        int max = (k.GetSignature()).size();
        return LinearSearchInconsistencyValue(k, max, GetMaxDistanceEncoding, config);
    }
    else if (config.measure_name == "sumdalal")
    {
        int max = k.size() * (k.GetSignature()).size();
        return LinearSearchInconsistencyValue(k, max, GetSumDistanceEncoding, config);
    }
    else
    {
        throw std::runtime_error("Unknown inconsistency measure");
    }
}
