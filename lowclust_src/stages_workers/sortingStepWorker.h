//
// Created by david on 12/01/16.
//

#ifndef LC_SORTINGSTEPWORKER_H
#define LC_SORTINGSTEPWORKER_H

#include "sortFasta.h"
#include "arguments.hh"
#include "workerInterface.h"
#include <vector>

class sortingStepWorker : public workerInterface {
private:
		char *input_buffer;
		char *output_buffer;
		unsigned block_size;

		bool terminateWorker(bool &rounds_remaining);
		std::size_t extractFastaName(char* query_block,
		                             SortFasta &rc);

//!! Needs to work for multiple line fasta files
		std::size_t createSortFasta(char *&query_block,
		                            SortFasta &rc);
		void writeToBuffer(char *output_buffer,
		                   const std::vector<SortFasta* > &fastas);
		void sortFasta();

public:
		void workerStage(const Arguments &args);

		sortingStepWorker(const Arguments &args);
		~sortingStepWorker();

		char* get_input_buffer();
		char* get_output_buffer();
};


#endif //LC_SORTINGSTEPWORKER_H
