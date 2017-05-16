//
// Created by david on 20/12/15.
//

#ifndef LC_SORTINGSTEP_CC_H
#define LC_SORTINGSTEP_CC_H

#include <pthread.h>
#include <string>
#include <fstream>

#include "semaphore_macros.h"
#include "stages_workers/sortingStepWorker.h"
#include "ioArguments.h"
#include "stepInterface.h"
#include <queue>

struct reader_struct{
		std::string leftover_name;
		std::string leftover_sequence;
		int leftover_length;

		void readSeqIntoBuffer(std::ifstream &stream, char *buffer, unsigned block_size);
};

	class sortingStep : public stepInterface{

	private:
			/* SEMAPHORES */
// Counting semaphores used to alert the io threads that they have requests
// to satisfy
			SEM_T reader;
			SEM_T writer;
// Mutex lock around all IO operations so that only one disk operation can
// happen at a time
			SEM_T IO;
// Counting semaphores used to ensure that the network never pops the queue
// too many times
			SEM_T input_ready_sem;
			SEM_T output_empty;
// Signal flags which detail if a specific IO operation has finished in
// it's entirety
			SEM_T finished_reading_sem;
			SEM_T finished_IO;
// Mutexes to restrict access to the queues
			SEM_T input_buf;
			SEM_T empty_input_buf;
			SEM_T output_buf;
			SEM_T empty_output_buf;
// Mutexes for the counting variables
			SEM_T read_input_sem;
			SEM_T written_output_sem;

			/* CLUSTER INFORMATION */
			int clusterSize;
			int workerSize;

			/* IO COUNTERS */
			int read_input = 0;
			int written_output = 0;
			int sent_input = 0;
			int recieved_output = 0;

			bool finished_reading = false;

			int num_allocated = 0;

			/* IO STRUCTURES */
			std::queue<int> input_bufs;
			std::queue<int> output_bufs;
			std::queue<int> empty_input_bufs;
			std::queue<int> empty_output_bufs;
			char **outputBuffers;
			char **inputBuffers;
			reader_struct r;

			IOArguments *ioargs;

			// Deliverables
			std::vector<std::string> mergedFiles;
			//std::string path;

			//sortingStep(sortingStep const&);                        // Don't Implement
			//void operator=(sortingStep const&);                     // Don't implement

// Termination conditions for the threads
			bool readerTermination(std::ifstream &input_stream);
			bool writerTermination();
			bool networkTermination();

// Network helper functions
			void initializeRecvs(MPI_Request *requests, unsigned block_size);
			void initializeSends(unsigned block_size);
			void reinitializeIrecv(int index, MPI_Request *requests, unsigned block_size);
			void sendMoreInput(int index, unsigned block_size);

// Helper functions for setting up structures
			void initializeSemaphores();
			void destroySemaphores();
			void terminateCluster(MPI_Request *requests, bool rounds_remaining);

// Input Output Constructors and Destructors
			void setupOutputBuffers(unsigned block_size);
			void releaseOutputBuffers();

			void setupInput(std::ifstream &input_stream, unsigned block_size);
			void releaseInput();

// Thread functions to do a specific IO task
			void readerFunction();
			void writerFunction();
			void networkFunction();

	public:
			// Constructor
			sortingStep(const Arguments &args,
			            int c_size,
			            int w_size);

			// Destructor
			~sortingStep();

			// deliverables
			std::vector<std::string> get_mergedFiles();
			//std::string get_directory();
	};

#endif //LC_SORTINGSTEP_CC_H
