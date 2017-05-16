#include "externalsort.hh"

std::vector<std::size_t> numSequencesPerFile; // from the extern in externalsort.hh, used in lowclust.cc
using namespace std;

fasta::Record *prepareHeap(const vector<std::string> &filenames,
                           vector<istream_iterator<fasta::Record> > &f_iterators,
                           vector<ifstream *> &ifstream_for_filenames,
                           istream_iterator<fasta::Record> &eos,
                           vector<pair<int, fasta::Record *> > &fasta_heap,
                           pair<int, fasta::Record *> &fasta_pair) {

	// Open an istream_iterator for each fasta file
	for (int i = 0; i < filenames.size(); i++) {
		// Make sure to keep the file stream "alive" outside this loop
		ifstream *f_str = new ifstream(filenames[i].c_str());
		f_iterators.push_back(istream_iterator<fasta::Record>(*f_str));
		ifstream_for_filenames.push_back(f_str);
	}

	// Heapify
	fasta::Record *plain_fastas = new fasta::Record[filenames.size()]; // used for copying.

	for (int i = 0; i < filenames.size(); i++) {
		if (f_iterators[i] != eos) {
			plain_fastas[i] = *(f_iterators[i]++); // copy first record from istream
			fasta_pair = std::make_pair(i, &(plain_fastas[i]));
			fasta_heap.push_back(fasta_pair);

			int ii = i;
			while (ii > 0) {

				int parent = (ii - 1) / 2;

				if (fasta_heap[parent].second->length() < fasta_pair.second->length()) {

					fasta_heap[ii] = fasta_heap[parent];
					fasta_heap[parent] = fasta_pair;

					ii = (ii - 1) / 2;

				} else {
					break;
				}
			}

		} else {
			std::cout << "External Sort exit" << std::endl;
			exit(1);
		}
	}
	return plain_fastas;
}

std::vector<std::string> conductHeapsort(vector<istream_iterator<fasta::Record> > &f_iterators,
                                         vector<pair<int, fasta::Record *> > &fasta_heap,
                                         istream_iterator<fasta::Record> &eos,
                                         fasta::Record *plain_fastas,
                                         unsigned block_size,
                                         const std::string &tmpdir) {


	std::string randstr = generate_directory_name("LowClustMerge", tmpdir);
	TempFiles listptr = TempFiles(tmpdir, randstr + "LowClustMerge", "merged", ".fasta");
	listptr.clear();

	std::ofstream outputfile(listptr.nextFileName().c_str());
	std::set<std::string> names;

	unsigned int usedBytes = 0;
	unsigned long rank = 0;
	std::size_t numSeqsInFile = 0;
	while (!fasta_heap.empty()) {

		// pop
		fasta::Record *fasta_ptr = fasta_heap[0].second;

		// update root element, otherwise remove root and heapify.
		if (f_iterators[fasta_heap[0].first] != eos) {
			plain_fastas[fasta_heap[0].first] = *(f_iterators[fasta_heap[0].first]++);
			fasta_heap[0].second = &plain_fastas[fasta_heap[0].first];
		} else {
			fasta_heap[0] = fasta_heap[fasta_heap.size() - 1];
			fasta_heap.pop_back();
		}

		// heapify
		siftDown(fasta_heap);

		if (names.find(fasta_ptr->name()) == names.end()) {
			names.insert(fasta_ptr->name());
			unsigned long next_size = fasta_ptr->get_print_size(rank);

			if (next_size + usedBytes > block_size) {

				numSequencesPerFile.push_back(numSeqsInFile);
				numSeqsInFile = 0;

				outputfile.close();
				outputfile.open(listptr.nextFileName().c_str());

				usedBytes = 0;
			}

			fasta_ptr->print(outputfile, rank);
			numSeqsInFile++;
			rank++;
			usedBytes += next_size;
		}
	}

	numSequencesPerFile.push_back(numSeqsInFile);

	numSequencesPerFile[0] = numSequencesPerFile[0] - 1;
	numSequencesPerFile[numSequencesPerFile.size() - 1] = numSequencesPerFile[numSequencesPerFile.size()
	                                                                          - 1] + 1;
	// Close last block file
	outputfile.close();
	return listptr.getFileNames();
}

std::vector<std::string> mergeAndBlockSortFasta(const vector<std::string> &filenames,
                                                unsigned block_size,
                                                const std::string &tmpdir) {

	vector<istream_iterator<fasta::Record> > f_iterators;
	vector<ifstream *> ifstream_for_filenames;
	istream_iterator<fasta::Record> eos;
	vector<pair<int, fasta::Record *> > fasta_heap;
	pair<int, fasta::Record *> fasta_pair;

	fasta::Record *plain_fastas = prepareHeap(filenames, f_iterators, ifstream_for_filenames,
	                                          eos, fasta_heap, fasta_pair);

	std::vector<std::string> mergedFilenames = conductHeapsort(f_iterators, fasta_heap, eos,
	                                                           plain_fastas, block_size,
	                                                           tmpdir);

	// Cleanup
	f_iterators.clear();
	for(vector<ifstream *>::iterator it = ifstream_for_filenames.begin(); it!= ifstream_for_filenames.end(); ++it){
		(*it)->close();
		delete *it;
	}
	return mergedFilenames;
}

void siftDown(vector<pair<int, fasta::Record *> > &fasta_heap) {

	int ii=0;
	unsigned long heap_size = fasta_heap.size();

	pair<int, fasta::Record *> tmp;

	while (true) {

		int l = 2*ii+1;
		int r = 2*ii+2;
		int min;

		if ( heap_size <= r) {
			if ( heap_size <= l ) {
				break;
			} else {
				min = l;
			}
		} else {
			min = ( fasta_heap[l].second->length() > fasta_heap[r].second->length()) ? l : r;
		}

		if (fasta_heap[ii].second->length() < fasta_heap[min].second->length()) {
			// swap
			tmp = fasta_heap[ii];
			fasta_heap[ii] = fasta_heap[min];
			fasta_heap[min] = tmp;
			ii = min;
		} else {
			break;
		}

	}
}
