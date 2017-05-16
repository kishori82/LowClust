//
// Created by david on 12/01/16.
//

#include <mpi.h>
#include "suffixArraysStepWorker.h"

// The termination condition is a buffer of length CHECK_LENGTH filled with
// the "*" characters. If the entire buffer is star characters then the
// algorithm has finished and we don't need the suffixArraysStepWorker anymore.
bool suffixArraysStepWorker::terminateWorker(bool &rounds_remaining){
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

void suffixArraysStepWorker::workerStage(const Arguments &args){
	bool rounds_remaining = true;

	while (true) {

		int rank = 0;
		// Receive the rank of the file to become a database
		MPI_Recv(&rank, 1, MPI_INT, MASTER,
		         RANK_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//std::cout << "RECEIVED RANK WORKER : " << rank << std::endl;

		// Wait for input to come in
		MPI_Recv(input_buffer, block_size, MPI_CHAR, MASTER,
		         INPUT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		if (terminateWorker(rounds_remaining)) {
			//std::cout << "TERMINATING WORKER" << std::endl;
			break;
		} else {

			//std::cout << "Creating the suffix array" << std::endl;
			Lastdb database(args);

			// Create a lastdb object using all of the buffers given
			database.createLastDatabase(input_buffer, database.get_dbs2(),
			                            database.get_db_lengths2());

			//std::cout << "Created the suffix array" << std::endl;

			//std::cout << "Sending the suffix array" << std::endl;
			sendFormattedDatabase(database, rank);
			//std::cout << "Sent the suffix array" << std::endl;

			// cleanup
			memset(input_buffer, 0, block_size);
			memset(output_buffer, 0, block_size * WRITER_MULT);
		}
	}
}

void suffixArraysStepWorker::sendFormattedDatabase(const Lastdb &database,
                                                   int rank){

	// Send the rank being sent
	//std::cout << "SENDING RANK WORKER : " << rank << std::endl;
	MPI_Send(&rank, 1, MPI_INT, MASTER,
	         RANK_TAG, MPI_COMM_WORLD);

	// Send the text files
	for(int i=0; i<7; i++) {
		std::string filename = database.get_db_files(i);
		//std::cout << "filename : " << filename << std::endl;
		long length = database.get_db_lengths()[filename];
		char *buffer = database.get_dbs()[filename];

		// Send the length
		//std::cout << "sending length : " << length << std::endl;

		MPI_Send(&length, 1, MPI_INT, MASTER,
		         LENGTH_TAG, MPI_COMM_WORLD);

		// Send the database file
		MPI_Send(buffer, length, MPI_CHAR, MASTER,
		         OUTPUT, MPI_COMM_WORLD);
		//std::cout << "Successful suffixArraysStepWorker send of database file : " << filename << std::endl;
	}
}

suffixArraysStepWorker::suffixArraysStepWorker(const Arguments &args){
	try {
		block_size = args.get_block_mb() * 1024 * 1024;
		input_buffer = new char[block_size]();
		output_buffer = new char[block_size*WRITER_MULT]();
	}catch(...){
		std::cout << "bad allocation of buffers in suffixArraysStepWorker" << std::endl;
	}
}

suffixArraysStepWorker::~suffixArraysStepWorker(){
	delete[] input_buffer;
	delete[] output_buffer;
}

char* suffixArraysStepWorker::get_input_buffer(){
	return input_buffer;
}

char* suffixArraysStepWorker::get_output_buffer(){
	return output_buffer;
}
