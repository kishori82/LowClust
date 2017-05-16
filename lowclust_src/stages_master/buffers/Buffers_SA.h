#ifndef BUFFERS_SA
#define BUFFERS_SA

#include <lastdb.hh>

class Buffers_SA {
private:
		void releaseOutputBuffers();
		void releaseInput();
		void setupInputBuffers();
		void setupOutputBuffers();

		int workerSize;
		unsigned block_size;

public:
		Buffers_SA(int ws,
		           unsigned bs);
		~Buffers_SA();

		char** inputBuffers;
		Lastdb **outputBuffers;
		int *file_lengths;
		int *ranks;
};

#endif
