//
// Created by david on 13/02/16.
//

#ifndef LC_BUFFERS_CLUSTERING_H
#define LC_BUFFERS_CLUSTERING_H

#include <iostream>

class Buffers_Clustering {

private:
		void releaseOutputBuffers();
		void releaseInput();
		void setupInputBuffers();
		void setupOutputBuffers(unsigned long num_seqs);

		int workerSize;
		unsigned block_size;
		std::size_t ns;


public:
		Buffers_Clustering(int ws,
		                   unsigned bs,
		                   unsigned long num_seqs);
		~Buffers_Clustering();

		//void printClusteredStatus();
		//void writeClusteredStatus(int i=0);


		char** inputBuffers;
		char** outputBuffers;
		char *repBufInput;
		char *repBufOutput;
		bool *clusteredStatus;
};


#endif //LC_BUFFERS_CLUSTERING_H
