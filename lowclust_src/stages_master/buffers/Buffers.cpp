//
// Created by david on 27/01/16.
//

#include <cstring>
#include <exception>
#include <iostream>
#include <stages_workers/workerInterface.h>
#include "Buffers.h"

Buffers::Buffers(unsigned ws,
                 unsigned bs):
		workerSize(ws),
		block_size(bs){
	try {
		setupInputBuffers();
		setupOutputBuffers();
	} catch (const std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << "Error allocation buffers" << std::endl;
	}
}

Buffers::~Buffers(){
	releaseOutputBuffers();
	releaseInput();
}

void Buffers::setupInputBuffers(){
	inputBuffers = new char *[workerSize];
	for(int i=0; i<workerSize; i++){
		inputBuffers[i] = new char[block_size];
		memset(inputBuffers[i], 0, block_size);
	}
}

void Buffers::setupOutputBuffers(){
	outputBuffers = new char *[workerSize];
	for(int i=0; i<workerSize; i++){
		outputBuffers[i] = new char[block_size * WRITER_MULT];
		memset(outputBuffers[i], 0, block_size * WRITER_MULT);
	}
}

void Buffers::releaseOutputBuffers(){
	for(int i=0; i<workerSize; i++){
		delete[] outputBuffers[i];
	}
	delete[] outputBuffers;
}

void Buffers::releaseInput(){
	for(int i=0; i<workerSize; i++){
		delete[] inputBuffers[i];
	}
	delete[] inputBuffers;
}
