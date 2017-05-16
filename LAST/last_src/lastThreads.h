//
// Created by david on 20/02/16.
//

#ifndef LC_LASTTHREADS_H
#define LC_LASTTHREADS_H


#include <Match.h>
#include <vector>
#include <queue>
#include <stages_master/Representatives.h>
#include "last_semaphores.h"
#include "globals.h"
#include "SegmentPairPot.hh"
#include "AlignmentPot.hh"
#include "GappedXdropAligner.hh"
#include "Centroid.hh"

using namespace cbrc;
struct threadData {

		GappedXdropAligner gappedXdropAligner;
		Centroid *centroid;

		std::queue<MultiSequence *> queryQueue;
		MultiSequence *query;

		std::queue<std::vector<Match*> *> outputVectorQueue;
		std::vector<Match*> *outputVector;

		int identifier;
		int round;
		Representatives r;

		semaphores_last_thread tsems;

		pthread_t thread;

		static void* threadEntry(void *args);

		void startThread();

		void threadFunction();

		void joinThread();

		threadData(int identifier,
		           const Representatives &reps);
		~threadData();

		// Find query matches to the suffix array, and do gapless extensions
		void alignGapless(SegmentPairPot &gaplessAlns, char strand);

		// Do gapped extensions of the gapless alignments
		void alignGapped(AlignmentPot &gappedAlns, SegmentPairPot &gaplessAlns, Phase::Enum phase);

		// Print the gapped alignments, after optionally calculating match
		// probabilities and re-aligning using the gamma-centroid algorithm
		void alignFinish(const AlignmentPot &gappedAlns, char strand);

		// Scan one batch of query sequences against one database volume
		void scan(char strand);
};


#endif //LC_LASTTHREADS_H
