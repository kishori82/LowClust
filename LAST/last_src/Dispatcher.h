//
// Created by david on 20/02/16.
//

#ifndef LC_DISPATCHER_H
#define LC_DISPATCHER_H

#include "globals.h"
#include "SegmentPair.hh"
#include "SequenceFormat.hh"
#include "Alphabet.hh"
#include "TwoQualityScoreMatrix.hh"
#include "ScoreMatrix.hh"

struct Dispatcher {

		const uchar *reference;  // the reference sequence
		const uchar *query;  // the query sequence
		const uchar *i;  // the reference quality data
		const uchar *j;  // the query quality data
		const ScoreMatrixRow *p;  // the query PSSM
		const ScoreMatrixRow *m;  // the score matrix
		const TwoQualityScoreMatrix &t;
		int d;  // the maximum score drop
		int z;
		Alphabet *aa;


		Dispatcher(Phase::Enum e, MultiSequence *text, MultiSequence *query,
		           ScoreMatrix *scoreMatrix, TwoQualityScoreMatrix *twoQualityScoreMatrix,
		           TwoQualityScoreMatrix *twoQualityScoreMatrixMasked,
		           sequenceFormat::Enum referenceFormat, Alphabet *alph) :

				reference(text->seqReader()),
				query(query->seqReader()),
				i(text->qualityReader()),
				j(query->qualityReader()),
				p(query->pssmReader()),
				m((e < args.maskLowercase) ?
				  scoreMatrix->caseSensitive : scoreMatrix->caseInsensitive),
				t((e < args.maskLowercase) ?
				  *twoQualityScoreMatrixMasked : *twoQualityScoreMatrix),
				d((e == Phase::gapless) ? args.maxDropGapless :
				  (e == Phase::gapped) ? args.maxDropGapped : args.maxDropFinal),
				z((args.inputFormat == sequenceFormat::fasta) ? 0 :
				  (referenceFormat == sequenceFormat::fasta) ? 1 : 2),
				aa(aa = alph) { }

		void shrinkToLongestIdenticalRun(SegmentPair &sp);

		int forwardGaplessScore(indexT x, indexT y) const ;

		int reverseGaplessScore(indexT x, indexT y) const ;

		indexT forwardGaplessEnd(indexT x, indexT y, int s) const ;

		indexT reverseGaplessEnd(indexT x, indexT y, int s) const ;

		bool isOptimalGapless(indexT x, indexT e, indexT y) const ;

		int gaplessScore(indexT x, indexT e, indexT y) const;
};

#endif //LC_DISPATCHER_H
