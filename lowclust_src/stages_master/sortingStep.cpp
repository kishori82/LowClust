//
// Created by david on 20/12/15.
//

#include "tempfiles.hh"
#include "utilities.hh"
#include "externalsort.hh"
#include "sortingStep.h"

bool sortingStep::readerTermination(std::ifstream &input_stream) {
	bool term = false;
	if (!input_stream.good()) {
		SEM_WAIT(finished_reading_sem);
		finished_reading = true;
		SEM_POST(finished_reading_sem);
		term = true;
	}
	return term;
}

bool sortingStep::writerTermination(){
	bool term = false;
	SEM_WAIT(finished_reading_sem);
	SEM_WAIT(read_input_sem);
	SEM_WAIT(written_output_sem);
	if (finished_reading && read_input == written_output) {
		term = true;
		SEM_POST(finished_IO);
	}
	SEM_POST(read_input_sem);
	SEM_POST(written_output_sem);
	SEM_POST(finished_reading_sem);
	return term;
}

bool sortingStep::networkTermination(){
	bool term = false;
	SEM_WAIT(finished_reading_sem);
	SEM_WAIT(read_input_sem);
	if(finished_reading && sent_input == read_input){
		term = true;
	}
	SEM_POST(read_input_sem);
	SEM_POST(finished_reading_sem);
	return term;
}

//!! This struture needs to be rewritten
// dont need the strings, just use pointers
void reader_struct::readSeqIntoBuffer(std::ifstream &stream,
                                      char *buffer,
                                      unsigned block_size){
	fasta::Record rc;
	std::size_t curr_size = 0;
	std::string accumulator;

	while (stream.good()) {
		number_of_sequences++;
		if(leftover_length == 0){
			stream >> rc;
			std::size_t length = (rc.length() + rc.name().size()) + 2;

			if(curr_size + length > block_size){
				leftover_name =  rc.name();
				leftover_sequence =  rc.sequence();
				leftover_length =  rc.length();
				break;
			}else{
				accumulator += (rc.name() + "\n" + rc.sequence() + "\n");
				curr_size += length;
			}
		}else{
			fasta::Record tmp(leftover_name, leftover_sequence, leftover_length);
			rc = tmp;
			leftover_name = "";
			leftover_sequence = "";
			leftover_length = 0;
		}
	}

	sprintf(buffer, "%s", accumulator.c_str());
}

void sortingStep::readerFunction(){
	// needs to be the sorted block
	setupInput(ioargs->reader, ioargs->block_size);

	bool term = false;
	do{
		SEM_WAIT(reader);
		SEM_WAIT(IO);

		SEM_WAIT(empty_input_buf);
		int index = empty_input_bufs.front();
		empty_input_bufs.pop();
		char* tmp = inputBuffers[index];
		SEM_POST(empty_input_buf);

		if (ioargs->reader.good()) {
			r.readSeqIntoBuffer(ioargs->reader, tmp, ioargs->block_size);
			SEM_WAIT(input_buf);
			input_bufs.push(index);
			SEM_POST(input_buf);
			SEM_WAIT(read_input_sem);
			read_input++;
			SEM_POST(read_input_sem);
		}
		term = readerTermination(ioargs->reader);

		SEM_POST(input_ready_sem);
		SEM_POST(IO);
	} while(!term);

	pthread_exit(0);
}

void sortingStep::writerFunction(){

	workerSize = clusterSize - 1;
	setupOutputBuffers(ioargs->block_size);

	// Create a temp directory structure
	std::string randstr = generate_directory_name("LowClustSorting", ioargs->n_args.get_tmp_dir());
	TempFiles listptr = TempFiles(ioargs->n_args.get_tmp_dir(), randstr + "LowClustSorting", "sorted",
	                              ".fasta");
	//path = listptr.path();

	while( !writerTermination() ){
		SEM_WAIT(writer);
		SEM_WAIT(IO);

		// Obtain the output buffers with output in them
		int index = output_bufs.front();
		char *tmp = outputBuffers[index];
		output_bufs.pop();

		// Write the sorted block to disk
		std::ofstream o(listptr.nextFileName().c_str());
		o.write(tmp, strlen(tmp));

		// Clean up the buffer and put it back in the available pool
		memset(tmp, 0, ioargs->block_size);
		empty_output_bufs.push(index);
		SEM_WAIT(written_output_sem);
		written_output++;
		SEM_POST(written_output_sem);
		SEM_POST(output_empty);

		SEM_POST(IO);
	}

	// Merge and re-sort the files
	mergedFiles = mergeAndBlockSortFasta(listptr.getFileNames(),
	                                     ioargs->block_size, ioargs->n_args.get_tmp_dir());

	// Erase all of the temporary files created during the sorting stage
	//listptr.clear();

	releaseInput();
	releaseOutputBuffers();

	pthread_exit(0);
}

void sortingStep::networkFunction(){

	MPI_Request requests[workerSize];

	initializeRecvs(requests, ioargs->block_size);
	initializeSends(ioargs->block_size);

	while (!networkTermination()) {
		// Wait on a output (RECV) from the workers.
		int index = -1;
		MPI_Waitany(workerSize, requests, &index, MPI_STATUS_IGNORE);
		recieved_output++;

		// Give the output to the writer function to write out the
		// output and get the buffer ready again for reusage
		output_bufs.push(index);
		SEM_POST(writer);
		reinitializeIrecv(index, requests, ioargs->block_size);
		sendMoreInput(index, ioargs->block_size);
	}

	terminateCluster(requests, false);

	pthread_exit(0);
}

void sortingStep::initializeSemaphores(){
#ifdef __APPLE__
	sem_unlink("/reader");
  if ( ( reader = sem_open("/reader", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/writer");
  if ( ( writer = sem_open("/writer", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/IO");
  if ( ( IO = sem_open("/IO", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/finished_reading_sem");
  if ( ( finished_reading_sem = sem_open("/finished_reading_sem", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/finished_IO");
  if ( ( finished_IO = sem_open("/finished_IO", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/input_ready_sem");
  if ( ( input_ready_sem = sem_open("/input_ready_sem", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/output_empty");
  if ( ( output_empty = sem_open("/output_empty", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/input_buf");
  if ( ( input_buf = sem_open("/input_buf", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/empty_input_buf");
  if ( ( empty_input_buf = sem_open("/empty_input_buf", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/output_buf");
  if ( ( output_buf = sem_open("/output_buf", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/empty_output_buf");
  if ( ( empty_output_buf = sem_open("/empty_output_buf", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/read_input_sem");
  if ( ( read_input_sem = sem_open("/read_input_sem", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/written_output_sem");
  if ( ( written_output_sem = sem_open("/written_output_sem", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
#elif __linux
	SEM_INIT(reader, 0, 0);
	SEM_INIT(writer, 0, 0);
	SEM_INIT(IO, 0, 1);
	SEM_INIT(finished_reading_sem, 0, 1);
	SEM_INIT(finished_IO, 0, 0);
	SEM_INIT(input_ready_sem, 0, 0);
	SEM_INIT(output_empty, 0, 0);
	SEM_INIT(input_buf, 0, 1);
	SEM_INIT(empty_input_buf, 0, 1);
	SEM_INIT(output_buf, 0, 1);
	SEM_INIT(empty_output_buf, 0, 1);
	SEM_INIT(read_input_sem, 0, 1);
	SEM_INIT(written_output_sem, 0, 1);
#endif
}

void sortingStep::destroySemaphores() {
#ifdef __APPLE__
	sem_unlink("/reader");
  sem_unlink("/writer");
  sem_unlink("/IO");
  sem_unlink("/finished_reading_sem");
  sem_unlink("/finished_IO");
  sem_unlink("/input_ready_sem");
  sem_unlink("/output_empty");
  sem_unlink("/input_buf");
  sem_unlink("/empty_input_buf");
  sem_unlink("/output_buf");
  sem_unlink("/empty_output_buf");
  sem_unlink("/read_input_sem");
  sem_unlink("/written_output_sem");
#elif __linux
	sem_destroy(&reader);
	sem_destroy(&writer);
	sem_destroy(&IO);
	sem_destroy(&input_ready_sem);
	sem_destroy(&output_empty);
	sem_destroy(&finished_reading_sem);
	sem_destroy(&finished_IO);
	sem_destroy(&input_buf);
	sem_destroy(&empty_input_buf);
	sem_destroy(&output_buf);
	sem_destroy(&empty_output_buf);
	sem_destroy(&read_input_sem);
	sem_destroy(&written_output_sem);
#endif
}

void sortingStep::terminateCluster(MPI_Request *requests, bool rounds_remaining){

	// Wait until all of the output has been collected, only then can we exit.
	while (sent_input != recieved_output) {
		int index = -1;
		MPI_Waitany(workerSize, requests, &index, MPI_STATUS_IGNORE);
		output_bufs.push(index);
		recieved_output++;
		SEM_POST(writer);
	}

	// Cleanup of the algorithm run
	MPI_Request finalRequests[workerSize];
	char terminationBuffer[10];
	memset(terminationBuffer, 0, 10);
	if(rounds_remaining) {
		// this round is finished
		sprintf(terminationBuffer, "******");
	}else{
		// all rounds are finished
		sprintf(terminationBuffer, "*****@");
	}
	for(int i=0; i<workerSize; i++){
		MPI_Isend(terminationBuffer, 10, MPI_CHAR, i+1,
		          INPUT, MPI_COMM_WORLD, &finalRequests[i]);
	}

	// Wait to be sure that all of the workers recieved our termination signal
	MPI_Waitall(workerSize, finalRequests, MPI_STATUS_IGNORE);

	SEM_WAIT(finished_IO);
}

void sortingStep::initializeRecvs(MPI_Request *requests, unsigned block_size){
	for(int i=0; i<workerSize; i++){
		SEM_WAIT(output_empty);

		int index = empty_output_bufs.front();
		char* tmp = outputBuffers[index];
		empty_output_bufs.pop();

		if (tmp == NULL) {
			std::cerr << "initializerecvs null ptr" << std::endl;
		}

		MPI_Irecv(tmp, block_size * WRITER_MULT, MPI_CHAR, i + 1, OUTPUT,
		          MPI_COMM_WORLD, &requests[i]);
	}
}

void sortingStep::initializeSends(unsigned block_size){
	for(int i=0; i<workerSize; i++){
		SEM_WAIT(input_ready_sem);

		SEM_WAIT(input_buf);
		int index = input_bufs.front();
		char *tmp = inputBuffers[i];
		input_bufs.pop();
		SEM_POST(input_buf);

		if (tmp == NULL) {
			std::cerr << "initializesends null ptr" << std::endl;
		}

		MPI_Send(tmp, block_size, MPI_CHAR, i+1,
		         INPUT, MPI_COMM_WORLD);
		sent_input++;

		memset(tmp, 0, block_size);
		SEM_WAIT(empty_input_buf);
		empty_input_bufs.push(index);
		SEM_POST(empty_input_buf);

		SEM_POST(reader);

		bool term = false;
		SEM_WAIT(finished_reading_sem);
		if(finished_reading){
			if(sent_input == read_input){
				term = true;
			}
		}
		SEM_POST(finished_reading_sem);

		if(term){
			break;
		}
	}
}

void sortingStep::reinitializeIrecv(int index, MPI_Request *requests, unsigned block_size){
	SEM_WAIT(output_empty);
	int output_index = empty_output_bufs.front();
	empty_output_bufs.pop();
	char* output = outputBuffers[output_index];


	if (output == NULL) {
		std::cerr << "reinitializeirecv null ptr" << std::endl;
	}

	MPI_Irecv(output, block_size * WRITER_MULT, MPI_CHAR, index + 1, OUTPUT,
	          MPI_COMM_WORLD, &requests[index]);
}

void sortingStep::sendMoreInput(int index, unsigned block_size){
	// Send more input to the worker which just finished sending
	// back it's output
	SEM_WAIT(input_ready_sem);
	int input_index = input_bufs.front();
	input_bufs.pop();
	char *input = inputBuffers[input_index];

	if (input == NULL) {
		std::cerr << "reinitializeirecv null ptr" << std::endl;
	}

	MPI_Send(input, block_size, MPI_CHAR, index+1,
	         INPUT, MPI_COMM_WORLD);
	sent_input++;
	memset(input, 0, block_size);
	empty_input_bufs.push(input_index);
	SEM_POST(reader);
}

void sortingStep::setupOutputBuffers(unsigned block_size){
	outputBuffers = new char *[workerSize];
	for(int i=0; i<workerSize; i++){
		outputBuffers[i] = new char[block_size * WRITER_MULT];
		memset(outputBuffers[i], 0, block_size * WRITER_MULT);
		empty_output_bufs.push(i);
		SEM_POST(output_empty);
	}
}

void sortingStep::releaseOutputBuffers(){
	for(int i=0; i<workerSize; i++){
		delete[] outputBuffers[i];
	}
	delete[] outputBuffers;
}

void sortingStep::setupInput(std::ifstream &input_stream, unsigned block_size) {
	inputBuffers = new char *[workerSize];
	num_allocated = 0;
	for(int i=0; i<workerSize; i++){
		if (input_stream.good()) {
			inputBuffers[i] = new char[block_size];
			memset(inputBuffers[i], 0, block_size);
			num_allocated++;
			r.readSeqIntoBuffer(input_stream, inputBuffers[i], block_size);

			SEM_WAIT(input_buf);
			input_bufs.push(i);
			SEM_POST(input_buf);

			SEM_WAIT(read_input_sem);
			read_input++;
			SEM_POST(read_input_sem);

			if (!input_stream.good()) {
				SEM_WAIT(finished_reading_sem);
				finished_reading = true;
				SEM_POST(finished_reading_sem);
			}
			SEM_POST(input_ready_sem);
			if(finished_reading){
				break;
			}
		}
	}
}

void sortingStep::releaseInput(){
	for(int i=0; i<num_allocated; i++){
		delete[] inputBuffers[i];
	}
	delete[] inputBuffers;
}

sortingStep::sortingStep(const Arguments &args, int c_size, int w_size) :
	clusterSize(c_size), workerSize(w_size){

	initializeSemaphores();

	ioargs = new IOArguments(args, 1);
}

sortingStep::~sortingStep() {

	delete ioargs;

	destroySemaphores();
}

// deliverables
std::vector<std::string> sortingStep::get_mergedFiles(){
	return mergedFiles;
}

/*
std::string sortingStep::get_directory(){
	return path;
}
 */
