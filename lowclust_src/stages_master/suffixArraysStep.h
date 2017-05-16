//
// Created by david on 20/12/15.
//

#ifndef LC_SUFFIXARRAYSSTEP_H
#define LC_SUFFIXARRAYSSTEP_H

//#include "workers.hh"
#include "stages_workers/suffixArraysStepWorker.h"
#include "semaphores/semaphores_suffix.h"
#include "semaphore_macros.h"
#include "arguments.hh"
#include "stepInterface.h"
#include "ioArguments.h"
#include "buffers/Buffers_SA.h"
#include <queue>

class suffixArraysStep : public stepInterface {

		semaphores_suffix sems;

		int clusterSize;
		int workerSize;

		int read_input = 0;
		int written_output = 0;
		int sent_input = 0;
		int recieved_output = 0;

		bool finished_reading = false;

		int num_allocated = 0;

		std::queue<int> input_bufs;
		std::queue<int> output_bufs;
		std::queue<int> empty_input_bufs;
		std::queue<int> empty_output_bufs;

		Buffers_SA *bufs;
		char **inputBuffers;
		Lastdb **outputDatabases;
		int *file_lengths;
		int *ranks;

		IOArguments *ioargs;

		// Previous stage's deliverables
		std::vector<std::string> mergedFiles;
		// This stage's deliverables
		std::string databaseDirectoryName;

		void readerFunction();
		void writerFunction();
		void networkFunction();

		void readSortedBlockIntoBuffer(char *buffer,
		                               const std::string &filename);
		void releaseDatabaseBuffers();
		int setupInput(unsigned block_size);
		void releaseInput();
		void setupOutputBuffers();

		void sendMoreInput(int index, unsigned block_size,
		                   int rank);
		void reinitializeIrecv(int index, MPI_Request *requests);
		int initializeSends(unsigned block_size);
		void initializeRecvs(MPI_Request *requests);
		void recieveDatabaseFromWorkers(int index);
		int receiveDeliverablesFromWorkers(MPI_Request *requests);

		bool networkTermination();
		bool writerTermination();
		void terminateCluster(MPI_Request *requests,
		                      bool rounds_remaining);

public:
		suffixArraysStep(const Arguments &args,
		                 int c_size,
		                 int w_size,
		                 const std::vector<std::string> & filenames);
		~suffixArraysStep();

		std::string get_databaseDirectoryName();
};

#endif //LC_SUFFIXARRAYSSTEP_H
