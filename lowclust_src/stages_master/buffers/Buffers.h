//
// Created by david on 27/01/16.
//

#ifndef LC_BUFFERS_H
#define LC_BUFFERS_H

class Buffers{

private:
		void releaseOutputBuffers();
		void releaseInput();
		void setupInputBuffers();
		void setupOutputBuffers();

		unsigned workerSize;
		unsigned block_size;


public:
		Buffers(unsigned ws,
		        unsigned bs);
		~Buffers();

		char** inputBuffers;
		char** outputBuffers;
};


#endif //LC_BUFFERS_H
