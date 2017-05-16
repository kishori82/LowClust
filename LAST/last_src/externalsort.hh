#ifndef  _EXTERNAL_SORT
#define  _EXTERNAL_SORT

#include <string>
#include "tempfiles.hh"
#include <vector>
#include <math.h>
#include <utility>
#include <fasta.hh>
#include "utilities.hh"
#include "externalsort.hh"
#include <iterator>

extern std::vector<std::size_t> numSequencesPerFile;

typedef unsigned long long countT;

std::vector<std::string> mergeAndBlockSortFasta(const std::vector<std::string> &filenames,
                                                unsigned block_size,
                                                const std::string &tmpdir);

void siftDown(std::vector<std::pair<int, fasta::Record *> > &fasta_heap);


std::vector<std::string> conductHeapsort(std::vector<std::istream_iterator<fasta::Record> > &f_iterators,
                                         std::vector<std::pair<int, fasta::Record *> > &fasta_heap,
                                         std::istream_iterator<fasta::Record> &eos,
                                         fasta::Record *plain_fastas,
                                         unsigned block_size,
                                         const std::string &tmpdir);

fasta::Record *prepareHeap(const std::vector<std::string> &filenames,
                           std::vector<std::istream_iterator<fasta::Record> > &f_iterators,
                           std::vector<std::ifstream *> &ifstream_for_filenames,
                           std::istream_iterator<fasta::Record> &eos,
                           std::vector<std::pair<int, fasta::Record *> > &fasta_heap,
                           std::pair<int, fasta::Record *> &fasta_pair);

#endif // _EXTERNAL_SORT
