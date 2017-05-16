//
// Created by david on 20/02/16.
//

#include "lastThreads.h"
#include "Dispatcher.h"
#include "DiagonalTable.hh"
#include "utilities.hh"

void threadData::alignGapless(SegmentPairPot &gaplessAlns, char strand) {

	Dispatcher dis(Phase::gapless, text, query, scoreMatrix, twoQualityScoreMatrix,
	               twoQualityScoreMatrixMasked, referenceFormat, alph);
	DiagonalTable dt;  // record already-covered positions on each diagonal
	countT matchCount = 0, gaplessExtensionCount = 0, gaplessAlignmentCount = 0;

	for (indexT i = 0; i < query->finishedSize(); i += args.queryStep) {

		for (unsigned x = 0; x < numOfIndexes; ++x) {

			const indexT *beg;
			const indexT *end;

			suffixArray.match(beg, end, dis.query + i, dis.reference, args.oneHitMultiplicity,
			                  args.minHitDepth);
			matchCount += end - beg;

			// Tried: if we hit a delimiter when using contiguous seeds, then
			// increase "i" to the delimiter position.  This gave a speed-up
			// of only 3%, with 34-nt tags.

			indexT gaplessAlignmentsPerQueryPosition = 0;

			for ( ; beg < end; ++beg) { // loop over suffix-array matches

				if (gaplessAlignmentsPerQueryPosition == args.maxGaplessAlignmentsPerQueryPosition) break;

				indexT j = *beg;  // coordinate in the reference sequence

				if (dt.isCovered(i, j)) continue;

				int fs = dis.forwardGaplessScore(j, i);
				int rs = dis.reverseGaplessScore(j, i);
				int score = fs + rs;
				++gaplessExtensionCount;

				// Tried checking the score after isOptimal & addEndpoint, but
				// the number of extensions decreased by < 10%, and it was
				// slower overall.
				if (score < minScoreGapless) continue;

				indexT tEnd = dis.forwardGaplessEnd(j, i, fs);
				indexT tBeg = dis.reverseGaplessEnd(j, i, rs);
				indexT qBeg = i - (j - tBeg);
				if (!dis.isOptimalGapless(tBeg, tEnd, qBeg)) continue;
				SegmentPair sp(tBeg, qBeg, tEnd - tBeg, score);

				if (args.outputType == 1) {  // we just want gapless alignments
					Alignment aln(identifier);
					aln.fromSegmentPair(sp);

					aln.writeIdentity(*text, *query, strand, args.isTranslated(),
					                  *alph, *outputVector, sim_cutoff);



				}
				else {
					gaplessAlns.add(sp);  // add the gapless alignment to the pot
				}

				++gaplessAlignmentsPerQueryPosition;
				++gaplessAlignmentCount;
				dt.addEndpoint(sp.end2(), sp.end1());
			}
		}
	}
	LOG(identifier << " initial matches=" << matchCount);
	LOG(identifier << " gapless extensions=" << gaplessExtensionCount);
	LOG(identifier << " gapless alignments=" << gaplessAlignmentCount);
}

void threadData::alignGapped(AlignmentPot &gappedAlns, SegmentPairPot &gaplessAlns, Phase::Enum phase) {

	Dispatcher dis(phase, text, query, scoreMatrix, twoQualityScoreMatrix, twoQualityScoreMatrixMasked,
	               referenceFormat, alph);
	indexT frameSize = args.isTranslated() ? (query->finishedSize() / 3) : 0;
	countT gappedExtensionCount = 0, gappedAlignmentCount = 0;

	// Redo the gapless extensions, using gapped score parameters.
	// Without this, if we self-compare a huge sequence, we risk getting
	// huge gapped extensions.
	for (size_t i = 0; i < gaplessAlns.size(); ++i) {
		SegmentPair &sp = gaplessAlns.items[i];

		int fs = dis.forwardGaplessScore(sp.beg1(), sp.beg2());
		int rs = dis.reverseGaplessScore(sp.beg1(), sp.beg2());
		indexT tEnd = dis.forwardGaplessEnd(sp.beg1(), sp.beg2(), fs);
		indexT tBeg = dis.reverseGaplessEnd(sp.beg1(), sp.beg2(), rs);
		indexT qBeg = sp.beg2() - (sp.beg1() - tBeg);
		sp = SegmentPair(tBeg, qBeg, tEnd - tBeg, fs + rs);

		if (!dis.isOptimalGapless(tBeg, tEnd, qBeg)) {
			SegmentPairPot::mark(sp);
		}
	}

	erase_if(gaplessAlns.items, SegmentPairPot::isMarked);

	gaplessAlns.cull(args.cullingLimitForGaplessAlignments);
	gaplessAlns.sort();  // sort by score descending, and remove duplicates

	LOG(identifier << " redone gapless alignments=" << gaplessAlns.size());

	for (size_t i = 0; i < gaplessAlns.size(); ++i) {
		SegmentPair &sp = gaplessAlns.get(i);

		if (SegmentPairPot::isMarked(sp)) continue;

		Alignment aln(identifier);
		AlignmentExtras extras;  // not used
		aln.seed = sp;

		dis.shrinkToLongestIdenticalRun(aln.seed);

		// do gapped extension from each end of the seed:
		// third...
		aln.makeXdrop(gappedXdropAligner, *centroid, dis.reference, dis.query, args.globality,
		              dis.m, scoreMatrix->maxScore, gapCosts, dis.d,
		              args.frameshiftCost, frameSize, dis.p,
		              dis.t, dis.i, dis.j, *alph, extras);
		++gappedExtensionCount;

		if (aln.score < args.minScoreGapped) continue;

		if (!aln.isOptimal(dis.reference, dis.query, args.globality, dis.m, dis.d, gapCosts,
		                   args.frameshiftCost, frameSize, dis.p,
		                   dis.t, dis.i, dis.j)) {
			// If retained, non-"optimal" alignments can hide "optimal"
			// alignments, e.g. during non-redundantization.
			continue;
		}

		gaplessAlns.markAllOverlaps(aln.blocks);
		gaplessAlns.markTandemRepeats(aln.seed, args.maxRepeatDistance);

		if (phase == Phase::final) gappedAlns.add(aln);
		else SegmentPairPot::markAsGood(sp);

		++gappedAlignmentCount;
	}

	LOG(identifier << " gapped extensions=" << gappedExtensionCount);
	LOG(identifier << " gapped alignments=" << gappedAlignmentCount);
}

void threadData::alignFinish(const AlignmentPot &gappedAlns, char strand) {

	LOG(identifier << " finishing...");
	for (size_t i = 0; i < gappedAlns.size(); ++i) {
		const Alignment &aln = gappedAlns.items[i];
		aln.writeIdentity(*text, *query, strand, args.isTranslated(),
		                  *alph, *outputVector, sim_cutoff);
	}
}

void threadData::scan(char strand) {

	LOG(identifier << " scanning...");

	SegmentPairPot gaplessAlns;
	alignGapless(gaplessAlns, strand);

	AlignmentPot gappedAlns;

	alignGapped(gappedAlns, gaplessAlns, Phase::final);

	if (args.outputType > 2) {  // we want non-redundant alignments
		gappedAlns.eraseSuboptimal();
		LOG(identifier << " nonredundant gapped alignments=" << gappedAlns.size());
	}

	gappedAlns.sort();  // sort by score
	alignFinish(gappedAlns, strand);
}

threadData::threadData(int identifier,
                       const Representatives &rep) : r(rep)
{
	for (int j = 0; j < 2; j++) {
		outputVectorQueue.push(new std::vector<Match*>());
	}

	MultiSequence *query1 = new MultiSequence();
	MultiSequence *query2 = new MultiSequence();

	queryAlph = alph;
	query1->initForAppending(1);
	query2->initForAppending(1);
	queryAlph->tr(query1->seqWriter(), query1->seqWriter() + query1->unfinishedSize());
	queryAlph->tr(query2->seqWriter(), query2->seqWriter() + query2->unfinishedSize());

	queryQueue.push(query1);
	queryQueue.push(query2);

	idInputQueue.push(identifier);

	centroid = new Centroid(gappedXdropAligner);
	this->identifier = identifier;

	round = 0;
}

threadData::~threadData(){
	delete centroid;

	for (int j = 0; j < 2; j++) {
		if (!outputVectorQueue.empty()) {
			delete outputVectorQueue.front();
			outputVectorQueue.pop();
		}
	}
}

void *threadData::threadEntry(void *args) {
	((threadData *) args)->threadFunction();
	return NULL;
}

void threadData::startThread() {
	pthread_create(&thread, NULL, threadEntry, this);
}

void threadData::joinThread() {
	pthread_join(thread, NULL);
}

struct repSorted {

		/*
		bool operator()(const Match &lhs, const Match &rhs) {
			// smallest to largest sorting
			if (lhs.rrank < rhs.rrank) return true;
			if (lhs.rrank > rhs.rrank) return false;
			if (lhs.similarity < rhs.similarity) return true;
			if (lhs.similarity > rhs.similarity) return false;

			return false;
		}
		 */
		bool operator()(const Match *lhs, const Match *rhs) {
			// smallest to largest sorting
			if (lhs->rrank < rhs->rrank) return true;
			if (lhs->rrank > rhs->rrank) return false;
			if (lhs->similarity < rhs->similarity) return true;
			if (lhs->similarity > rhs->similarity) return false;

			return false;
		}
} comp;

void threadData::threadFunction() {
	while (true) {

		SEM_WAIT(tsems.readSema);

		if (threadTerminate) {
			break;
		}

		SEM_WAIT(tsems.writeSema);

		outputVector = outputVectorQueue.front();
		outputVectorQueue.pop();
		query = queryQueue.front();
		queryQueue.pop();

		scan('+');

		// Parse the best match (representative) for each member
		// hence, for each query, sort by representative
		//!! isnt working as intended

		if (r.repClustering) {
			sort(outputVector->begin(), outputVector->end(), comp);

			parseReps(outputVector, r);

		} else {
			sort(outputVector->begin(), outputVector->end());
			selectBest(outputVector, r);
		}

		query->reinitForAppending();

		SEM_WAIT(sems->inputOutputQueueSema);
		idInputQueue.push(identifier);
		inputQueue.push(query);
		idOutputQueue.push(identifier);
		outputQueue.push(outputVector);
		SEM_POST(sems->inputOutputQueueSema);
		SEM_POST(sems->readerSema);
		SEM_POST(sems->writerSema);
	}
	pthread_exit(0);
}
