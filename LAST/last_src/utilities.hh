#ifndef ___UTILITIES___
#define ___UTILITIES___
#include <iostream>

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <stages_master/Representatives.h>

#include "Match.h"

char * split_n_pick(const std::string  &strn,  char *buf, char d, unsigned int n);

std::string random_str(const int len);

// Generate random string for output in order to allow mutiple LAST+ binaries to run simultaneously on a single machine.
// Check if the directory structure already exists. If it does we need to generate a new randstr
std::string generate_directory_name(const char *base, const std::string &tmpdir);
void selectBest(std::vector<Match*> *outputVector, const Representatives &reps);
void parseReps(std::vector<Match*> *outputVector, const Representatives &reps);
//void selectBest(std::vector<Match> *outputVector, bool *repStatuses);

void faststrcat(char* dst, unsigned long dstLength, char* src, unsigned long srcLength);
void faststrcatString(char* dst, unsigned long dstLength, std::string src);

#endif //_UTILITIES

