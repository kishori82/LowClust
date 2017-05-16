//
// Created by david on 13/02/16.
//

#ifndef LC_SEMAPHORES_SUFFIX_H
#define LC_SEMAPHORES_SUFFIX_H


#include <semaphores.hh>

class semaphores_suffix {
private:
		void initializeSemaphores();
		void destroySemaphores();
public:
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
		SEM_T round_done;
// Mutexes to restrict access to the queues
		SEM_T input_buf;
		SEM_T empty_input_buf;
		SEM_T output_buf;
		SEM_T empty_output_buf;
// Mutexes for the counting variables
		SEM_T read_input_sem;
		SEM_T written_output_sem;

		semaphores_suffix();
		~semaphores_suffix();

};


#endif //LC_SEMAPHORES_SUFFIX_H
