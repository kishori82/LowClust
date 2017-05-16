//
// Created by david on 12/01/16.
//

#include <iostream>
#include <mpi.h>
#include "sortingStepWorker.h"
#include <cstring> //strlen
#include <algorithm> //sort

bool sortingStepWorker::terminateWorker(bool &rounds_remaining){
	bool termination = true;
	// Check if this round is finished
	for(int i=0; i<5; i++){
		if(input_buffer[i] != '*'){
			termination = false;
		}
	}
	// Check if all rounds are finished
	if(termination){
		if(input_buffer[5] == '@'){
			rounds_remaining = false;
		}
	}
	return termination;
};

std::size_t sortingStepWorker::extractFastaName(char* query_block, SortFasta &rc) {

	// Find the first instance of the tab or newline character
	char c = 0;
	std::size_t length = 0;

	rc.name_start = query_block;
	rc.name_end = query_block;

	//std::cout << "START" << std::endl;
	while (c != '\t' && c != '\n'){
		c = query_block[length];
		length++;
		rc.name_end++;
	}

	// continue until we hit the newline
	while ( c != '\n'){
		c = query_block[length];
		length++;
	}

	return length;
}

//!! Needs to work for multiple line fasta files
std::size_t sortingStepWorker::createSortFasta(char *&query_block, SortFasta &rc) {

	std::size_t nameSize = extractFastaName(query_block, rc);
	query_block += nameSize;

	rc.seq_start = query_block;
	rc.seq_end = query_block;

	std::size_t sequenceSize = 0;
	char c = 0;
	while ( c != '\n' ) {
		c = query_block[sequenceSize];
		if (c == '>') break;  // we have hit the next FASTA sequence
		sequenceSize++;
		rc.seq_end++;
	}

	rc.set_length(sequenceSize);

	query_block += sequenceSize;
	return sequenceSize + nameSize;
}


void sortingStepWorker::writeToBuffer(char *output_buffer,
                                      const std::vector<SortFasta* > &fastas){
	size_t count = 0;
	for(size_t i=0; i<fastas.size(); i++) {
		// write out the name
		char *tmp = fastas[i]->name_start;
		while (tmp != fastas[i]->name_end) {
			output_buffer[count] = *tmp;
			count++;
			tmp++;
		}

		// write out the sequence
		char *tmp2 = fastas[i]->seq_start;
		while (tmp2 != fastas[i]->seq_end) {
			output_buffer[count] = *tmp2;
			count++;
			tmp2++;
		}

		delete fastas[i];
	}
}

void sortingStepWorker::sortFasta(){

	std::size_t length = strlen(input_buffer);
	unsigned read_so_far = 0;
	char* tmp = input_buffer;
	std::vector<SortFasta*> fastas;

	while (read_so_far < length) {
		SortFasta *rc = new SortFasta();
		read_so_far += createSortFasta(tmp, *rc);

		fastas.push_back(rc);
	}

	std::sort(fastas.rbegin(), fastas.rend(), compSortFastaPointerLT);

	// Write out the sorted fasta into the output buffer
	writeToBuffer(output_buffer, fastas);

	std::cout << "Finished writing sorted buffer to output" << std::endl;
}

void sortingStepWorker::workerStage(const Arguments &args){
	bool rounds_remaining = true;

	/*
		if(args.get_verbosity()){
			std::cout << "Sorting Round beginning" << std::endl;
		}
	 */

	while (true) {
		// Wait for input to come in
		MPI_Recv(input_buffer, block_size, MPI_CHAR, MASTER,
		         INPUT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		if (terminateWorker(rounds_remaining)) {
			break;
		} else {

			// sort the input buffer and place it in the output buffer
			sortFasta();

			// Send the output to the master
			MPI_Send(output_buffer, block_size * WRITER_MULT, MPI_CHAR, MASTER,
			         OUTPUT, MPI_COMM_WORLD);

			// cleanup
			memset(input_buffer, 0, block_size);
			memset(output_buffer, 0, block_size * WRITER_MULT);
		}
	}
}

sortingStepWorker::sortingStepWorker(const Arguments &args){
	try {
		block_size = args.get_block_mb() * 1024 * 1024;
		input_buffer = new char[block_size]();
		output_buffer = new char[block_size*WRITER_MULT]();
	}catch(...){
		std::cout << "bad allocation of buffers in sortingStepWorker" << std::endl;
	}
}

sortingStepWorker::~sortingStepWorker(){
	delete[] input_buffer;
	delete[] output_buffer;
}

char* sortingStepWorker::get_input_buffer(){
	return input_buffer;
}

char* sortingStepWorker::get_output_buffer(){
	return output_buffer;
}
