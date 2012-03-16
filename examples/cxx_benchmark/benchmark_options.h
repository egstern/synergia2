#ifndef BENCHMARK_OPTIONS_H_
#define BENCHMARK_OPTIONS_H_
#include <string>

// this file was automatically generated by the command
//     synergia benchmark_options.py --create-cxx-options-source
// DO NOT EDIT

struct Benchmark_options
{
    Benchmark_options(int argc, char **argv);
    int partpercell;
    int verbosity;
    bool avoid;
    int gridy;
    int gridx;
    int gridz;
    bool autotune;
    int efieldcomm;
    int chargecomm;
    bool diagnostics;
    int sortperiod;
};
#endif /* BENCHMARK_OPTIONS_H_ */
