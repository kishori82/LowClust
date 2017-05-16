//
// Created by david on 20/02/16.
//

#include "Dispatcher.h"
#include "gaplessTwoQualityXdrop.hh"
#include "gaplessPssmXdrop.hh"
#include "gaplessXdrop.hh"


// Shrink the SegmentPair to its longest run of identical matches.
// This trims off possibly unreliable parts of the gapless alignment.
// It may not be the best strategy for protein alignment with subset
// seeds: there could be few or no identical matches...
void Dispatcher::shrinkToLongestIdenticalRun(SegmentPair &sp) {
	sp.maxIdenticalRun(reference, query, aa->canonical);
	sp.score = gaplessScore(sp.beg1(), sp.end1(), sp.beg2());
}

int Dispatcher::forwardGaplessScore(indexT x, indexT y) const {
	if (z == 0) return forwardGaplessXdropScore(reference + x, query + y, m, d);
	if (z == 1) return forwardGaplessPssmXdropScore(reference + x, p + y, d);
	return forwardGaplessTwoQualityXdropScore(reference + x, i + x, query + y, j + y, t, d);
}

int Dispatcher::reverseGaplessScore(indexT x, indexT y) const {
	if (z == 0) return reverseGaplessXdropScore(reference + x, query + y, m, d);
	if (z == 1) return reverseGaplessPssmXdropScore(reference + x, p + y, d);
	return reverseGaplessTwoQualityXdropScore(reference + x, i + x, query + y, j + y, t, d);
}

indexT Dispatcher::forwardGaplessEnd(indexT x, indexT y, int s) const {
	if (z == 0) return forwardGaplessXdropEnd(reference + x, query + y, m, s) - reference;
	if (z == 1) return forwardGaplessPssmXdropEnd(reference + x, p + y, s) - reference;
	return forwardGaplessTwoQualityXdropEnd(reference + x, i + x, query + y, j + y, t, s) - reference;
}

indexT Dispatcher::reverseGaplessEnd(indexT x, indexT y, int s) const {
	if (z == 0) return reverseGaplessXdropEnd(reference + x, query + y, m, s) - reference;
	if (z == 1) return reverseGaplessPssmXdropEnd(reference + x, p + y, s) - reference;
	return reverseGaplessTwoQualityXdropEnd(reference + x, i + x, query + y, j + y, t, s) - reference;
}

bool Dispatcher::isOptimalGapless(indexT x, indexT e, indexT y) const {
	if (z == 0) return isOptimalGaplessXdrop(reference + x, reference + e, query + y, m, d);
	if (z == 1) return isOptimalGaplessPssmXdrop(reference + x, reference + e, p + y, d);
	return isOptimalGaplessTwoQualityXdrop(reference + x, reference + e, i + x, query + y, j + y, t,
	                                       d);
}

int Dispatcher::gaplessScore(indexT x, indexT e, indexT y) const {
	if (z == 0) return gaplessAlignmentScore(reference + x, reference + e, query + y, m);
	if (z == 1) return gaplessPssmAlignmentScore(reference + x, reference + e, p + y);
	return gaplessTwoQualityAlignmentScore(reference + x, reference + e, i + x, query + y, j + y, t);
}
