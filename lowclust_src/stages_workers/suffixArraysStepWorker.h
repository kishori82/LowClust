//
// Created by david on 12/01/16.
//

#ifndef LC_SUFFIXARRAYSSTEPWORKER_H
#define LC_SUFFIXARRAYSSTEPWORKER_H


#include "lastdb.hh"
#include "arguments.hh"
#include "workerInterface.h"

class suffixArraysStepWorker : public workerInterface {

private:
		char *input_buffer;
		char *output_buffer;
		unsigned block_size;

		bool terminateWorker(bool &rounds_remaining);

		void sendFormattedDatabase(const Lastdb &database,
		                           int rank);

public:
		void workerStage(const Arguments &args);

		suffixArraysStepWorker(const Arguments &args);
		~suffixArraysStepWorker();

		// Getters
		char* get_input_buffer();
		char* get_output_buffer();
};


#endif //LC_SUFFIXARRAYSSTEPWORKER_H
