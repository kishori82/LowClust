//
// Created by david on 28/12/15.
//

#ifndef LC_STEPINTERFACE_H
#define LC_STEPINTERFACE_H

#include <pthread.h>
#include "mpi.h"
#include "arguments.hh"

/*
 * Abstract class for all the steps in the algorithm
 */

class stepInterface {

private:
		/* THREAD STRUCTURES */
		pthread_t r_thread;
		pthread_t w_thread;
		pthread_t n_thread;

		static void* readerEntry(void *args);
		static void* writerEntry(void *args);
		static void* networkEntry(void *args);

		virtual void readerFunction() = 0;
		virtual void writerFunction() = 0;
		virtual void networkFunction() = 0;

		void startThreads();
		void joinThreads();

public:

		void runStep();

		/*
		virtual stepInterface(const Arguments &args,
		            unsigned c_size,
		            unsigned w_size);
		            */

		/*
		virtual ~stepInterface();
		 */
};

#endif //LC_STEPINTERFACE_H
