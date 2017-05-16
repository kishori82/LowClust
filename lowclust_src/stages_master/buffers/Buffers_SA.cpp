//
// Created by david on 03/02/16.
//

#include "Buffers_SA.h"

Buffers_SA::Buffers_SA(int ws,
                       unsigned bs) :
		workerSize(ws),
		block_size(bs){
	try {

		file_lengths = new int[workerSize];
		ranks = new int[workerSize];

		setupInputBuffers();
		setupOutputBuffers();
	} catch (const std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << "Error allocation buffers" << std::endl;
	}
}

Buffers_SA::~Buffers_SA(){

	delete[] file_lengths;
	delete[] ranks;

	releaseOutputBuffers();
	releaseInput();
}

void Buffers_SA::setupInputBuffers(){
	inputBuffers = new char *[workerSize];
	for(int i=0; i<workerSize; i++){
		inputBuffers[i] = new char[block_size];
		memset(inputBuffers[i], 0, block_size);
	}
}

void Buffers_SA::setupOutputBuffers(){
	outputBuffers = new Lastdb *[workerSize];
	for(int i=0; i<workerSize; i++){
		outputBuffers[i] = new Lastdb();
	}
}

void Buffers_SA::releaseOutputBuffers(){
	for(int i=0; i<workerSize; i++){
		outputBuffers[i]->clear();
	}
	delete[] outputBuffers;
}

void Buffers_SA::releaseInput(){
	for(int i=0; i<workerSize; i++){ //for(int i=0; i<num_allocated; i++){
		delete[] inputBuffers[i];
	}
	delete[] inputBuffers;
}
