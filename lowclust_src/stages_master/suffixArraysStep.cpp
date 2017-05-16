//
// Created by david on 20/12/15.
//

#include "utilities.hh"
#include "suffixArraysStep.h"
#include <sstream>

void suffixArraysStep::readerFunction(){

	int filesRead = setupInput(ioargs->block_size);

	for(int i=filesRead; i<mergedFiles.size(); i++){
		SEM_WAIT(sems.reader);
		SEM_WAIT(sems.IO);

		SEM_WAIT(sems.empty_input_buf);
		int index = empty_input_bufs.front();
		empty_input_bufs.pop();
		char* tmp = inputBuffers[index];
		SEM_POST(sems.empty_input_buf);

		//std::cout << mergedFiles[i] << std::endl;
		readSortedBlockIntoBuffer(tmp, mergedFiles[i]);

		if( i == mergedFiles.size()-1){
			SEM_WAIT(sems.finished_reading_sem);
			finished_reading = true;
			SEM_POST(sems.finished_reading_sem);
		}

		SEM_WAIT(sems.input_buf);
		input_bufs.push(index);
		SEM_POST(sems.input_buf);
		SEM_WAIT(sems.read_input_sem);
		read_input++;
		SEM_POST(sems.read_input_sem);

		SEM_POST(sems.input_ready_sem);
		SEM_POST(sems.IO);
	}

	//std::cout << "EXITED READER" << std::endl;
	pthread_exit(0);
}


void suffixArraysStep::writerFunction(){

	setupOutputBuffers();

	// Create a directory to put all of the databases in.
	std::string randstr = generate_directory_name("LowClustDatabases", ioargs->n_args.get_tmp_dir());
	TempFiles listptr = TempFiles(ioargs->n_args.get_tmp_dir(), randstr + "LowClustDatabases", "block",
	                              "");
	listptr.generateDirectory();

	databaseDirectoryName = ioargs->n_args.get_tmp_dir() + "/" + randstr + "LowClustDatabases";
	//!!
	//std::cout << databaseDirectoryName << std::endl;

	for(int i=0; i<mergedFiles.size(); i++){
		SEM_WAIT(sems.writer);
		SEM_WAIT(sems.IO);

		// Obtain the databases
		int index = output_bufs.front();
		Lastdb *tmp = outputDatabases[index];
		output_bufs.pop();

		// Write out the databases
		//!!
		//std::cout << listptr.nextFileName() << std::endl;
		tmp->writeDatabase(databaseDirectoryName, "");

		// Clean up the buffer and put it back in the available pool
		empty_output_bufs.push(index);
		SEM_WAIT(sems.written_output_sem);
		written_output++;
		SEM_POST(sems.written_output_sem);
		SEM_POST(sems.output_empty);

		SEM_POST(sems.IO);
		writerTermination();
	}

	releaseInput();
	releaseDatabaseBuffers();
	delete[] file_lengths;
	delete[] ranks;

	//std::cout << "EXITED WRITER" << std::endl;
	pthread_exit(0);
}

int suffixArraysStep::receiveDeliverablesFromWorkers(MPI_Request *requests){
	// Wait on a output (RECV) from the workers.
	int index = -1;
	MPI_Waitany(workerSize, requests, &index, MPI_STATUS_IGNORE);

	recieveDatabaseFromWorkers(index);
	//std::cout << "Recievied database : " << rank << std::endl;
	recieved_output++;

	// Give the output to the writer function to write out the
	// output and get the buffer ready again for reusage
	output_bufs.push(index);
	SEM_POST(sems.writer);

	return index;
}

void suffixArraysStep::networkFunction(){

	MPI_Request requests[workerSize];
	file_lengths = new int[workerSize];
	ranks = new int[workerSize];

	initializeRecvs(requests);
	int sent_files = initializeSends(ioargs->block_size);

	while (!networkTermination()) {
		int index = receiveDeliverablesFromWorkers(requests);

		reinitializeIrecv(index, requests);
		sendMoreInput(index, ioargs->block_size, sent_files);
		sent_files++;
	}
	//int index = receiveDeliverablesFromWorkers(requests);
	receiveDeliverablesFromWorkers(requests);

	terminateCluster(requests, false);

	//std::cout << "EXITED NETWORK" << std::endl;
	pthread_exit(0);
}


int suffixArraysStep::setupInput(unsigned block_size){
	inputBuffers = new char *[workerSize];
	int numRead = 0;
	for(int i=0; i<workerSize; i++){
		if(i == mergedFiles.size()){
			SEM_WAIT(sems.finished_reading_sem);
			finished_reading = true;
			SEM_POST(sems.finished_reading_sem);
			break;
		}

		inputBuffers[i] = new char[block_size];
		memset(inputBuffers[i], 0, block_size);
		readSortedBlockIntoBuffer(inputBuffers[i], mergedFiles[i]);
		num_allocated++;

		SEM_WAIT(sems.input_buf);
		input_bufs.push(i);
		SEM_POST(sems.input_buf);

		SEM_WAIT(sems.read_input_sem);
		read_input++;
		SEM_POST(sems.read_input_sem);
		numRead++;
		SEM_POST(sems.input_ready_sem);
	}
	return numRead;
}


void suffixArraysStep::recieveDatabaseFromWorkers(int index){

	Lastdb *d = outputDatabases[index];
	int rank = ranks[index];

	d->clear();

	// Set the individual file names for the database partition
	std::stringstream s;
	s << "block" << rank;
	d->set_db_files(s.str()+".prj", 0);
	d->set_db_files(s.str()+".des", 1);
	d->set_db_files(s.str()+".bck", 2);
	d->set_db_files(s.str()+".sds", 3);
	d->set_db_files(s.str()+".ssp", 4);
	d->set_db_files(s.str()+".tis", 5);
	d->set_db_files(s.str()+".suf", 6);

	for(int i=0; i<7; i++){
		// Recieve size of the database file

		MPI_Recv(&file_lengths[index], 1, MPI_INT, index + 1, LENGTH_TAG,
		         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Set the length
		d->set_db_length(file_lengths[index],d->get_db_files(i));

		// Setup a memory buffer for the database file
		d->get_dbs2()[d->get_db_files(i)] = new char[file_lengths[index]];
		// Recieve the database file
		MPI_Recv(d->get_dbs2()[d->get_db_files(i)], file_lengths[index], MPI_CHAR, index + 1,
		         OUTPUT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	std::cout << "Received database rank : " << rank << std::endl;
}

void suffixArraysStep::readSortedBlockIntoBuffer(char* buffer, const std::string &filename){
	std::ifstream file(filename.c_str());
	file.seekg(0, std::ios::end);
	std::ifstream::pos_type filesize = file.tellg();
	file.seekg(0, std::ios::beg);
	file.read(buffer, filesize);

	//printf("%s", buffer);
}


void suffixArraysStep::releaseDatabaseBuffers(){
	for(int i=0; i<workerSize; i++){
		outputDatabases[i]->clear();
	}
	delete[] outputDatabases;
}

void suffixArraysStep::terminateCluster(MPI_Request *requests,
                                        bool rounds_remaining) {
	// Wait until all of the output has been collected, only then can we exit.
	while(sent_input != recieved_output){
		//int index = receiveDeliverablesFromWorkers(requests);
		receiveDeliverablesFromWorkers(requests);
		/*
		int index = -1;
		MPI_Waitany(workerSize, requests, &index, MPI_STATUS_IGNORE);

		//!! Why was this previously not here and everythign was fine?
		//!! Why does this thing basically not have any affect?
		//!! Probably because on a 2 core status, we never have a scenario
		// where this has to run?
		recieveDatabaseFromWorkers(index);
		recieved_output++;
		std::cout << "Received database rank : " << ranks[index] << std::endl;

		receiveClusteredRepresentatives(listptr, index);

		output_bufs.push(index);
		SEM_POST(sems.writer);
		 */
	}

	//std::cout << "SENDING TERMINATION SIGNALS" << std::endl;

	// Cleanup of the algorithm run
	char terminationBuffer[10];
	if(rounds_remaining) {
		// this round is finished
		sprintf(terminationBuffer, "******");
	}else{
		// all rounds are finished
		sprintf(terminationBuffer, "*****@");
	}

	// Send the termination signal
	int rank = 0;
	MPI_Request final_ranks[workerSize];
	MPI_Request final_inputs[workerSize];
	for(int i=0; i<workerSize; i++){
		MPI_Isend(&rank, 1, MPI_INT, i+1,
		          RANK_TAG, MPI_COMM_WORLD, &final_ranks[i]);
		MPI_Isend(terminationBuffer, 10, MPI_CHAR, i+1,
		          INPUT, MPI_COMM_WORLD, &final_inputs[i]);
	}

	// Wait to be sure that all of the workers recieved our termination signal
	MPI_Waitall(workerSize, final_ranks, MPI_STATUS_IGNORE);
	MPI_Waitall(workerSize, final_inputs, MPI_STATUS_IGNORE);

	//std::cout << "TERMINATED THE CLUSTER" << std::endl;

	SEM_WAIT(sems.finished_IO);
}


void suffixArraysStep::releaseInput(){
	for(int i=0; i<num_allocated; i++){
		delete[] inputBuffers[i];
	}
	delete[] inputBuffers;
}

void suffixArraysStep::setupOutputBuffers(){
	outputDatabases = new Lastdb *[workerSize];
	for(int i=0; i<workerSize; i++){
		empty_output_bufs.push(i);
		outputDatabases[i] = new Lastdb();
		SEM_POST(sems.output_empty);
	}
}

void suffixArraysStep::sendMoreInput(int index, unsigned block_size, int rank){

	// Send more input to the worker which just finished sending
	// back it's output
	SEM_WAIT(sems.input_ready_sem);
	int input_index = input_bufs.front();
	input_bufs.pop();
	char *input = inputBuffers[input_index];

	//int rank = 0;
	// Send the rank of the file
	MPI_Send(&rank, 1, MPI_INT, index+1,
	         RANK_TAG, MPI_COMM_WORLD);

	// Send the file
	MPI_Send(input, block_size, MPI_CHAR, index+1,
	         INPUT, MPI_COMM_WORLD);

	sent_input++;
	memset(input, 0, block_size);
	empty_input_bufs.push(input_index);
	SEM_POST(sems.reader);
}

void suffixArraysStep::reinitializeIrecv(int index, MPI_Request *requests){
	// Rank of the incoming database from a finished and reset worker
	MPI_Irecv(&ranks[index], 1, MPI_INT, index + 1,
	          RANK_TAG, MPI_COMM_WORLD, &requests[index]);
}

int suffixArraysStep::initializeSends(unsigned block_size){

	int rank = 0;
	for(int i=0; i<workerSize; i++){
		SEM_WAIT(sems.input_ready_sem);

		SEM_WAIT(sems.input_buf);
		int index = input_bufs.front();
		char *tmp = inputBuffers[i];
		input_bufs.pop();
		SEM_POST(sems.input_buf);

		// Send rank of the current file to become a database
		MPI_Send(&i, 1, MPI_INT, i+1,
		         RANK_TAG, MPI_COMM_WORLD);

		MPI_Send(tmp, block_size, MPI_CHAR, i+1,
		         INPUT, MPI_COMM_WORLD);
		rank++;
		sent_input++;

		memset(tmp, 0, block_size);
		SEM_WAIT(sems.empty_input_buf);
		empty_input_bufs.push(index);
		SEM_POST(sems.empty_input_buf);

		SEM_POST(sems.reader);

		bool term = false;
		SEM_WAIT(sems.finished_reading_sem);
		if(finished_reading){
			if(sent_input == read_input){
				term = true;
			}
		}
		SEM_POST(sems.finished_reading_sem);

		if (term) {
			break;
		}
	}

	return rank;
}

void suffixArraysStep::initializeRecvs(MPI_Request *requests){
	for(int i=0; i<workerSize; i++){
		SEM_WAIT(sems.output_empty);

		int index = empty_output_bufs.front();
		empty_output_bufs.pop();

		// Rank of the incoming database
		MPI_Irecv(&ranks[index], 1, MPI_INT, i + 1,
		          RANK_TAG, MPI_COMM_WORLD, &requests[i]);
	}
}

bool suffixArraysStep::networkTermination(){
	bool term = false;
	SEM_WAIT(sems.finished_reading_sem);
	SEM_WAIT(sems.read_input_sem);

	if(finished_reading && sent_input == read_input){
		term = true;
	}

	SEM_POST(sems.read_input_sem);
	SEM_POST(sems.finished_reading_sem);
	return term;
}

bool suffixArraysStep::writerTermination(){
	bool term = false;
	SEM_WAIT(sems.finished_reading_sem);
	SEM_WAIT(sems.read_input_sem);
	SEM_WAIT(sems.written_output_sem);
	if (finished_reading && read_input == written_output) {
		term = true;
		SEM_POST(sems.finished_IO);
	}
	SEM_POST(sems.read_input_sem);
	SEM_POST(sems.written_output_sem);
	SEM_POST(sems.finished_reading_sem);
	return term;
}

suffixArraysStep::suffixArraysStep(const Arguments &args,
                                   int c_size,
                                   int w_size,
                                   const std::vector<std::string> & filenames) :
		clusterSize(c_size), workerSize(w_size), mergedFiles(filenames){

	ioargs = new IOArguments(args, 1);

	bufs = new Buffers_SA(workerSize, ioargs->block_size);
}

suffixArraysStep::~suffixArraysStep() {
	delete ioargs;

	delete bufs;
}

std::string suffixArraysStep::get_databaseDirectoryName(){
	return databaseDirectoryName;
}
