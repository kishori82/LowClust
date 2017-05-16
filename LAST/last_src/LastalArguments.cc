// Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014 Martin C. Frith

#include "LastalArguments.hh"
#include "stringify.hh"
//#include <unistd.h>  // getopt
#include <algorithm>  // max
#include <iostream>
#include <vector>
#include <cmath>  // log
#include <cstring>  // strtok

#define ERR(x) throw std::runtime_error(x)

/*
static void badopt( char opt, const char* arg ){
  ERR( std::string("bad option value: -") + opt + ' ' + arg );
}
 */

namespace cbrc{

LastalArguments::LastalArguments() :
  outFile("-"),
  outputFormat(2), 
  outputType(3),
  strand(-1),  // depends on the alphabet
  globality(0),
  maskLowercase(-1),  // depends on the lowercase option used with lastdb
  minScoreGapped(-1),  // depends on the alphabet
  minScoreGapless(-1),  // depends on minScoreGapped and the outputType
  matchScore(-1),  // depends on the alphabet
  mismatchCost(-1),  // depends on the alphabet
  gapExistCost(-1),  // depends on the alphabet
  gapExtendCost(-1),  // depends on the alphabet
  insExistCost(-1),  // depends on gapExistCost
  insExtendCost(-1),  // depends on gapExtendCost
  gapPairCost(100000),  // I want it to be infinity, but avoid overflow
  frameshiftCost(-1),  // this means: ordinary, non-translated alignment
  matrixFile(""),
  maxDropGapped(-1),  // depends on minScoreGapped & maxDropGapless
  maxDropGapless(-1),  // depends on the score matrix
  maxDropFinal(-1),  // depends on maxDropGapped
  inputFormat(sequenceFormat::fasta),
  minHitDepth(0),  // depends on the outputType
  oneHitMultiplicity(10),
  maxGaplessAlignmentsPerQueryPosition(0),  // depends on oneHitMultiplicity
  cullingLimitForGaplessAlignments(0),
  queryStep(1),
  batchSize(0),  // depends on the outputType, and voluming
  maxRepeatDistance(1000),  // sufficiently conservative?
  temperature(-1),  // depends on the score matrix
  gamma(1),
  geneticCodeFile(""),
  verbosity(0),
  version(0),
  scoreCutoff(20),
  evalueCutoff(1.0e-6),
  threadNum(1),
	topHits(10){}

void LastalArguments::setDefaultsFromAlphabet( bool isDna, bool isProtein,
                                               bool isCaseSensitiveSeeds,
					       bool isVolumes ){

  if( strand < 0 ) strand = (isDna || isTranslated()) ? 2 : 1;

  if( outputType < 2 && minScoreGapped < 0 ) minScoreGapped = minScoreGapless;

  if( isProtein ){
    // default match & mismatch scores: Blosum62 matrix
    if( matchScore < 0 && mismatchCost >= 0 ) matchScore   = 1;  // idiot-proof
    if( mismatchCost < 0 && matchScore >= 0 ) mismatchCost = 1;  // idiot-proof
    if( gapExistCost   < 0 ) gapExistCost   =  11;
    if( gapExtendCost  < 0 ) gapExtendCost  =   2;
    if( minScoreGapped < 0 ) minScoreGapped = 100;
  }
  else if( !isQuality( inputFormat ) ){
    if( matchScore     < 0 ) matchScore     =   1;
    if( mismatchCost   < 0 ) mismatchCost   =   1;
    if( gapExistCost   < 0 ) gapExistCost   =   7;
    if( gapExtendCost  < 0 ) gapExtendCost  =   1;
    if( minScoreGapped < 0 ) minScoreGapped =  40;
  }
  else{  // sequence quality scores will be used:
    if( matchScore     < 0 ) matchScore     =   6;
    if( mismatchCost   < 0 ) mismatchCost   =  18;
    if( gapExistCost   < 0 ) gapExistCost   =  21;
    if( gapExtendCost  < 0 ) gapExtendCost  =   9;
    if( minScoreGapped < 0 ) minScoreGapped = 180;
    // With this scoring scheme for DNA, gapless lambda ~= ln(10)/10,
    // so these scores should be comparable to PHRED scores.
    // Furthermore, since mismatchCost/matchScore = 3, the target
    // distribution of paired bases ~= 99% identity.  Because the
    // quality scores are unlikely to be perfect, it may be best to
    // use a lower target %identity than we otherwise would.
  }

  if( insExistCost < 0 ) insExistCost = gapExistCost;
  if( insExtendCost < 0 ) insExtendCost = gapExtendCost;

  if( outputType < 2 ) minScoreGapless = minScoreGapped;

  if( maskLowercase < 0 ){
    if( isCaseSensitiveSeeds && inputFormat != sequenceFormat::pssm )
      maskLowercase = 2;
    else
      maskLowercase = 0;
  }

  if( batchSize == 0 ){
    // Without lastdb voluming, smaller batches can be faster,
    // probably because of the sorts done for gapped alignment.  With
    // voluming, we want the batches to be as large as will
    // comfortably fit into memory, because each volume gets read from
    // disk once per batch.
    if( !isVolumes )
      batchSize = 0x2000;  // 8 Kbytes (?)
    else if( inputFormat != sequenceFormat::fasta )
      batchSize = 0x100000;   // 1 Mbyte
    else if( outputType == 0 )
      batchSize = 0x1000000;  // 16 Mbytes
    else
      batchSize = 0x8000000;  // 128 Mbytes
    // (should we reduce the 128 Mbytes, for fewer out-of-memory errors?)
  }

  if( minHitDepth == 0 )
    minHitDepth = (outputType == 0) ? 1 : -1;

  if( maxGaplessAlignmentsPerQueryPosition == 0 )
    maxGaplessAlignmentsPerQueryPosition =
      (oneHitMultiplicity > 0) ? oneHitMultiplicity : -1;

  if( isTranslated() && frameshiftCost < gapExtendCost )
    ERR( "the frameshift cost must not be less than the gap extension cost" );

  if( insExistCost != gapExistCost || insExtendCost != gapExtendCost ){
    if( isTranslated() )
      ERR( "can't combine option -F with option -A or -B" );
  }
}

void LastalArguments::setDefaultsFromMatrix( double lambda ){
  if( temperature < 0 ) temperature = 1 / lambda;

  if( maxDropGapless < 0 ){  // should it depend on temperature or lambda?
    if( temperature < 0 ) maxDropGapless = 0;  // shouldn't happen
    else                  maxDropGapless = int( 10.0 * temperature + 0.5 );
  }

  if( maxDropGapped < 0 ){
    maxDropGapped = std::max( minScoreGapped - 1, maxDropGapless );
  }

  if( maxDropFinal < 0 ) maxDropFinal = maxDropGapped;
}

int LastalArguments::calcMinScoreGapless( double numLettersInReference,
					  double numOfIndexes ) const{
  if( minScoreGapless >= 0 ) return minScoreGapless;

  // ***** Default setting for minScoreGapless *****

  // This attempts to ensure that the gapped alignment phase will be
  // reasonably fast relative to the gapless alignment phase.

  // The expected number of gapped extensions per query position is:
  // kGapless * referenceSize * exp(-lambdaGapless * minScoreGapless).

  // The number of gapless extensions per query position is
  // proportional to: maxGaplessAlignmentsPerQueryPosition * numOfIndexes.

  // So we want exp(lambdaGapless * minScoreGapless) to be
  // proportional to: kGapless * referenceSize / (n * numOfIndexes).

  // But we crudely ignore kGapless.

  // The proportionality constant was guesstimated by some limited
  // trial-and-error.  It should depend on the relative speeds of
  // gapless and gapped extensions.

  if( temperature < 0 ) return minScoreGapped;  // shouldn't happen

  double n = maxGaplessAlignmentsPerQueryPosition;
  if( maxGaplessAlignmentsPerQueryPosition + 1 == 0 ) n = 10;  // ?
  double x = 1000.0 * numLettersInReference / (n * numOfIndexes);
  if( x < 1 ) x = 1;
  int s = int( temperature * std::log(x) + 0.5 );
  return std::min( s, minScoreGapped );
}

}  // end namespace cbrc
