//
// Created by david on 12/01/16.
//

#include <mpi.h>
#include <stages_master/Representatives.h>
#include "lastdb.hh"
#include "clusteringStepWorker.h"

// The termination condition is a buffer of length CHECK_LENGTH filled with
// the "*" characters. If the entire buffer is star characters then the
// algorithm has finished and we don't need the clusteringStepWorker anymore.
bool clusteringStepWorker::terminateWorker(bool &rounds_remaining){
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


void clusteringStepWorker::workerStage(const Arguments &args){

	bool rounds_remaining = true;
	std::size_t rounds = 0;
	while (rounds_remaining) {

		Lastdb db(args);
		db.recieveFormattedDatabase();

		Representatives reps;
		reps.receiveRepresentativeStatuses();

		Lastal al(db.get_const_dbs(), args);

		while (true) {
			MPI_Recv(input_buffer, block_size, MPI_CHAR, MASTER,
			         INPUT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


			if (terminateWorker(rounds_remaining)) {
				break;
			} else {
				al.lastAlign(input_buffer, output_buffer, 0, reps);

				// Send the output to the master
				MPI_Send(output_buffer, block_size * WRITER_MULT, MPI_CHAR, MASTER,
				         OUTPUT, MPI_COMM_WORLD);


				// cleanup
				memset(input_buffer, 0, block_size);
				memset(output_buffer, 0, block_size * WRITER_MULT);
			}
		}

		rounds++;
		reps.destroy();
	}
}

clusteringStepWorker::clusteringStepWorker(const Arguments &args){
	try {
		block_size = (unsigned)(args.get_block_mb() * 1024 * 1024);
		input_buffer = new char[block_size]();
		output_buffer = new char[block_size*WRITER_MULT]();
	}catch(...){
		std::cout << "bad allocation of buffers in clusteringStepWorker" << std::endl;
	}
}

clusteringStepWorker::~clusteringStepWorker(){
	delete[] input_buffer;
	delete[] output_buffer;
}

