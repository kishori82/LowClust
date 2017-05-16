//
// Created by david on 12/01/16.
//

#ifndef LC_WORKERINTERFACE_H
#define LC_WORKERINTERFACE_H

#include "arguments.hh"

#define REP_TAG 5
#define LENGTH_TAG 4
#define RANK_TAG 3
#define INPUT 2
#define OUTPUT 1
#define MASTER 0

#define WRITER_MULT 10

extern unsigned long number_of_sequences;

class workerInterface {


public:
		virtual void workerStage(const Arguments &args) = 0;

};


#endif //LC_WORKERINTERFACE_H
