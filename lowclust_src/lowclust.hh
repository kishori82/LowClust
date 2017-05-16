#ifndef LOWCLUST_HH
#define LOWCLUST_HH

#include "arguments.hh"
#include <vector>

void mpiSetup(int argc, char **argv);
bool checkCluster();
void distributedSuffixArrays(const Arguments &args);
void cluster(const Arguments &args);
void removeDirectories(const std::vector<std::string> &dirs);

#endif
