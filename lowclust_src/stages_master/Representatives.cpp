//
// Created by david on 15/02/16.
//

#include <mpi.h>
#include <stages_workers/workerInterface.h>
#include "Representatives.h"


void Representatives::receiveRepresentativeStatuses(){
  // Set this rounds index to be such.

	// Receive the broadcast: the size of the buffer
	MPI_Bcast(&numReps, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);

	// Receive the broadcast: the the index start
	MPI_Bcast(&indexStart, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);

	// Setup the clustered status buffer to give to lastal
	representatives = new bool[numReps];

  // Recieve the broadcast: clustered status buffer to give to lastal
	MPI_Bcast(representatives, numReps, MPI::BYTE, MASTER,
	          MPI_COMM_WORLD);
}

// for workers
Representatives::Representatives() :
		representatives(NULL),
		numReps(0),
		repClustering(false){

}

// for master
Representatives::Representatives(bool* reps,
                                 std::size_t is,
                                 std::size_t nr) :
		representatives(reps),
		indexStart(is),
		numReps(nr),
    repClustering(true)
{ }

// Need it because we lack shared pointers and relying on RAII will just destroy our structures...
void Representatives::destroy(){
	if(representatives != NULL) {
		delete representatives;
	}
}

Representatives::~Representatives(){
	/*
	if(representatives != NULL) {
		delete representatives;
	}
	 */
}
