#ifndef __LASTAL_HH
#define __LASTAL_HH

#include "LastalArguments.hh"

#include "QualityPssmMaker.hh"
#include "OneQualityScoreMatrix.hh"
#include "TwoQualityScoreMatrix.hh"
#include "qualityScoreUtil.hh"
//#include "LambdaCalculator.hh"
#include "GeneticCode.hh"
#include "SubsetSuffixArray.hh"
#include "Centroid.hh"
#include "GappedXdropAligner.hh"
#include "AlignmentPot.hh"
#include "Alignment.hh"
#include "SegmentPairPot.hh"
#include "SegmentPair.hh"
#include "ScoreMatrix.hh"
#include "Alphabet.hh"
#include "MultiSequence.hh"
#include "DiagonalTable.hh"
#include "GeneralizedAffineGapCosts.hh"
#include "gaplessXdrop.hh"
#include "gaplessPssmXdrop.hh"
#include "gaplessTwoQualityXdrop.hh"
#include "io.hh"
#include "stringify.hh"
//#include "lastex.hh"
//#include "LastexArguments.hh"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <ctime>
#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <string>
#include <vector>

#include <queue>

#include <cstdlib>
#include <cstdio>
#include <pthread.h>
#include "semaphores.hh"
#include "externalsort.hh"
#include "utilities.hh"
#include <sstream>

#include <arguments.hh>
#include "Match.h"
#include "last_semaphores.h"
#include <stages_master/Representatives.h>
#include "globals.h"

namespace {
	bool isCaseSensitiveSeeds = false;  // initialize it to an "error" value
	indexT minSeedLimit;
}

void readOuterPrj(unsigned &volumes,
                  indexT &seqCount,
                  indexT &seqLen,
                  indexT &delimiterNum,
                  indexT &bucketDepth,
                  const std::map<std::string, char *> &dbs,
                  std::size_t round);

void readIndex(indexT bucketDepth,
               indexT seqCount,
               indexT delimiterNum,
               const std::map<std::string, char *> &dbs,
               std::size_t round);

void *__writerFunction(void *arguments);

void readerFunction(char* query_block,
                    const std::map<std::string, char *> &dbs,
                    indexT bucketDepth,
                    indexT seqCount,
                    indexT delimiterNum,
                    std::size_t round);

void initializeWorkers(const Representatives &rep);

// Delete the global objects on the heap
void deleteStructures();

void createStructures(const std::map<std::string, char *> &dbs,
                      indexT &bucketDepth,
                      indexT &seqCount,
                      indexT &delimiterNum,
                      indexT &seqLen,
                      std::size_t round);

// Set up a scoring matrix, based on the user options
void makeScoreMatrix();

class Lastal {

private:
		std::map<std::string, char *> dbs;
		float similarity_cutoff;

public:
		Lastal(const std::map<std::string, char *> &lastDatabases,
		       const Arguments &args);
		//~Lastal();

		void lastAlign(
				char *queryblock,
				char *outputBuffer,
				std::size_t round,
				const Representatives &reps);
};

void lastal(const std::map<std::string, char *> &dbs,
            char *queryblock,
            char *outputBuffer,
            std::size_t round,
            float sim_cutoff);


#endif
