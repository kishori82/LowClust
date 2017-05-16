//
// Created by david on 20/02/16.
//

#ifndef LC_GLOBALS_H
#define LC_GLOBALS_H

#include "MultiSequence.hh"
#include "LastalArguments.hh"
#include "Alphabet.hh"
#include "ScoreMatrix.hh"
#include "TwoQualityScoreMatrix.hh"
#include "SubsetSuffixArray.hh"
#include "GeneralizedAffineGapCosts.hh"
#include "OneQualityScoreMatrix.hh"
#include "QualityPssmMaker.hh"
#include "GeneticCode.hh"
#include "last_semaphores.h"
#include <queue>
#include <Match.h>

#define ERR(x) throw std::runtime_error(x)
//#define LOG(x) if( args.verbosity > 0 ) std::cerr << "lastal: " << x << '\n'

#define LOG(x) \
  if( args.verbosity > 0 ){ \
    std::stringstream stream; \
    stream << "lastal: " << x << "\n"; \
		std::cerr << stream.str(); \
  }

//!! global sorting is required for now just make its single threaded one batch
#define INPUT_SIZE 9999999999
//#define INPUT_SIZE 10000

using namespace cbrc;

typedef MultiSequence::indexT indexT;
typedef unsigned long long countT;


extern LastalArguments args;

extern Alphabet *queryAlph;
extern Alphabet *alph;
extern MultiSequence *text;
extern ScoreMatrix *scoreMatrix;
extern TwoQualityScoreMatrix *twoQualityScoreMatrix;
extern TwoQualityScoreMatrix *twoQualityScoreMatrixMasked;
extern OneQualityScoreMatrix *oneQualityScoreMatrix;

extern GeneralizedAffineGapCosts gapCosts;

extern SubsetSuffixArray suffixArray;

extern unsigned numOfIndexes; // = 1;  // assume this value, if unspecified
extern int minScoreGapless;
extern float sim_cutoff;
extern sequenceFormat::Enum referenceFormat;

extern std::queue<int> idInputQueue;
extern std::queue<MultiSequence *> inputQueue;
extern std::queue<int> idOutputQueue;
extern std::queue<std::vector<Match*> *> outputQueue;

extern semaphores_last *sems;
extern bool threadTerminate;


namespace Phase {

	enum Enum {
			gapless, gapped, final
	};
}

#endif //LC_GLOBALS_H
