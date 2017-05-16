//
// Created by david on 13/02/16.
//

#include "Buffers_Clustering.h"
#include <stages_workers/workerInterface.h>
#include <cstring>
#include <fstream>

Buffers_Clustering::Buffers_Clustering(int ws,
                                       unsigned bs,
                                       unsigned long num_seqs) :
		workerSize(ws),
		block_size(bs),
		ns(num_seqs){
	try {
		setupInputBuffers();
		setupOutputBuffers(num_seqs);
	} catch (const std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << "Error allocation buffers" << std::endl;
	}
}

Buffers_Clustering::~Buffers_Clustering(){
	releaseOutputBuffers();
	releaseInput();
}

void Buffers_Clustering::setupInputBuffers(){
	inputBuffers = new char *[workerSize];
	for(int i=0; i<workerSize; i++){
		inputBuffers[i] = new char[block_size];
		memset(inputBuffers[i], 0, block_size);
	}
	repBufInput = new char[block_size];
}

void Buffers_Clustering::setupOutputBuffers(unsigned long num_seqs){

	outputBuffers = new char *[workerSize];
	for(int i=0; i<workerSize; i++){
		outputBuffers[i] = new char[block_size * WRITER_MULT];
		memset(outputBuffers[i], 0, block_size * WRITER_MULT);
	}

	repBufOutput = new char[block_size * WRITER_MULT];
	memset(repBufOutput, 0, block_size * WRITER_MULT);

	clusteredStatus = new bool[num_seqs];
}

void Buffers_Clustering::releaseOutputBuffers(){
	for(int i=0; i<workerSize; i++){
		delete[] outputBuffers[i];
	}
	delete[] outputBuffers;

	delete[] repBufOutput;
	delete[] clusteredStatus;
}

void Buffers_Clustering::releaseInput(){
	for(int i=0; i<workerSize; i++){
		delete[] inputBuffers[i];
	}
	delete[] inputBuffers;
}

/*
void Buffers_Clustering::writeClusteredStatus(int i){
	char num = '0'+i;
	std::string fname = "/home/david/statuses";
	fname.push_back(num);
	fname += ".txt";
	std::ofstream o(fname.c_str());
	for(std::size_t i=0; i<ns; i++){
		//std::cout << clusteredStatus[i];
		o << clusteredStatus[i] << "\n";
	}
	//std::cout << std::endl;
}

void Buffers_Clustering::printClusteredStatus()
{
	for(std::size_t i=0; i<ns; i++){
		std::cout << clusteredStatus[i];
	}
	std::cout << std::endl;
}
*/