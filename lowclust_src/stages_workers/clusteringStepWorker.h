//
// Created by david on 12/01/16.
//

#ifndef LC_CLUSTERINGSTEPWORKER_H
#define LC_CLUSTERINGSTEPWORKER_H

#include "lastal.hh"
#include "workerInterface.h"

class clusteringStepWorker : public workerInterface {

private:
		unsigned block_size;
        //!! representatives
        bool* bool_vector;

		bool terminateWorker(bool &rounds_remaining);

public:
		char *input_buffer;
		char *output_buffer;

		void workerStage(const Arguments &args);

		clusteringStepWorker(const Arguments &args);
		~clusteringStepWorker();

};


#endif //LC_CLUSTERINGSTEPWORKER_H
