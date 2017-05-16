// Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014 Martin C. Frith

#include "Alignment.hh"
#include "GeneticCode.hh"
#include "MultiSequence.hh"
#include "Alphabet.hh"

#include <iterator>  // ostream_iterator

//#include "lastex.hh"

// make C++ tolerable:
#define CI(type) std::vector<type>::const_iterator

using namespace cbrc;

static char *writeGaps(char *dest, size_t num) {
	char *end = dest + num;
	while (dest < end) {
		*dest++ = '-';
	}
	return dest;
}

// Write only the sequence names and percent identity for use with LowClust
//!! poor form but bigger fish to fry
char *start = new char[1000];
char *start2 = new char[1000];
void Alignment::writeIdentity(const MultiSequence &database,
                              const MultiSequence &queries,
                              char strand,
                              bool isTranslated,
                              const Alphabet &alph,
                              std::vector<Match*> &outputVector,
                              float similarity_cutoff) const {

	assert(!blocks.empty());
	indexT alnBeg1 = beg1();
	indexT representative = database.whichSequence(alnBeg1);

	indexT queries_size = queries.finishedSize();
	indexT frameSize2 = isTranslated ? (queries_size / 3) : 0;
	indexT alnBeg2 = aaToDna(beg2(), frameSize2);
	indexT member = queries.whichSequence(strand == '+' ? alnBeg2 : queries_size - alnBeg2);

	//char *start = new char[1000];
	//char *start2 = new char[1000];

	char *dest;
	char *dest2;

	dest = writeRepresentative(database.seqReader(), alph, frameSize2, start);
	dest2 = writeMember(queries.seqReader(), alph, frameSize2, start2);

	float identity = Match::calculateIdentity(queries.seqName(member), dest2-start2);
	unsigned long qrank = Match::calculateRank(queries.seqName(member));
	unsigned long rrank = Match::calculateRank(database.seqName(representative));

	//std::cout << identity << " " << similarity_cutoff*100 << std::endl;
	if (identity >= similarity_cutoff * 100) {
		// Write sequence names
		// Write the identity value
		Match *m = new Match(database.seqName(representative), queries.seqName(member),
		        dest-start, qrank, rrank, identity);

		outputVector.push_back(m);
	}

	//delete[] start;
	//delete[] start2;
}

static char *writeGaps(char *dest, Alignment::indexT num) {
	char *end = dest + num;
	while (dest < end) {
		*dest++ = '-';
	}
	return dest;
}

char* Alignment::writeRepresentative(const uchar *reference,
                                     const Alphabet &alph,
                                     indexT frameSize,
                                     char *reference_buffer) const {

	for (CI(SegmentPair) i = blocks.begin(); i < blocks.end(); ++i) {

		if (i > blocks.begin()) { // between each pair of aligned blocks:
			CI(SegmentPair) j = i - 1;

			// append unaligned chunk of top sequence:
			reference_buffer = alph.rtCopy(reference + j->end1(), reference + i->beg1(), reference_buffer);

			// append gaps for unaligned chunk of bottom sequence:
			//indexT gap2, frameshift2;
			std::size_t gap2, frameshift2;

			//!! Problems with types
			sizeAndFrameshift(j->end2(), i->beg2(), frameSize, gap2, frameshift2);
			if (frameshift2) *reference_buffer++ = '-';
			reference_buffer = writeGaps(reference_buffer, gap2);
		}

		// append aligned chunk of top sequence:
		reference_buffer = alph.rtCopy(reference + i->beg1(), reference + i->end1(), reference_buffer);
	}

	return reference_buffer;
}

char *Alignment::writeMember(const uchar *seq,
                             const Alphabet &alph,
                             indexT frameSize,
                             char *dest) const {

	for (CI(SegmentPair) i = blocks.begin(); i < blocks.end(); ++i) {
		if (i > blocks.begin()) { // between each pair of aligned blocks:
			CI(SegmentPair) j = i - 1;

			// append gaps for unaligned chunk of top sequence:
			dest = writeGaps(dest, i->beg1() - j->end1());

			//append unaligned chunk of bottom sequence:
			//!!
			//indexT gap2, frameshift2;
			std::size_t gap2, frameshift2;
			//!! Problems with types
			sizeAndFrameshift(j->end2(), i->beg2(), frameSize, gap2, frameshift2);
			if (frameshift2 == 1) *dest++ = '\\';
			if (frameshift2 == 2) *dest++ = '/';
			dest = alph.rtCopy(seq + i->beg2() - gap2, seq + i->beg2(), dest);
		}

		// append aligned chunk of bottom sequence:
		dest = alph.rtCopy(seq + i->beg2(), seq + i->end2(), dest);
	}

	return dest;
}
