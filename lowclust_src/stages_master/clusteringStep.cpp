//
// Created by david on 20/12/15.
//

#include "clusteringStep.h"
#include "lastdb.hh"



unsigned long total_num_read; // need to put this at the top

void clusteringStep::readerFunction(){
	pthread_exit(0);
}

void clusteringStep::writerFunction() {
	pthread_exit(0);
}

void clusteringStep::networkInit(std::size_t round, ClusterSet &cs)
{
	// Read and distribute the databases
	std::stringstream s;
	s << "block" << round;
	Lastdb db(s.str());
	db.readFormattedDatabase(databaseDirectoryName);
	db.distributeFormattedDatabase();

	// Self align to create clusters of representatives
	Lastal al(db.get_const_dbs(), ioargs->n_args);
	repClustering(al, round, cs);

	//std::cout << "NSR : " << num_read << std::endl;
	sendRepresentatives(nr[round], cs);
}

int clusteringStep::networkInit2(std::size_t round, MPI_Request *requests){

	// Setup the input for all workers
	int filesRead = setupInput(mergedFiles, round);

	initializeRecvs(requests, ioargs->block_size);
	initializeSends(ioargs->block_size);
	return filesRead;
}

void clusteringStep::gatherRemains(MPI_Request *requests,
                                   unsigned block_size,
                                   ClusterSet &cs){
	while(sent_input != recieved_output){
		int index = -1;
		MPI_Waitany(workerSize, requests, &index, MPI_STATUS_IGNORE);
		char* output = bufs->outputBuffers[index];
		cs.parseBuffer(output, bufs->clusteredStatus, total_num_read);
		recieved_output++;
		memset(output, 0, block_size * WRITER_MULT);
	}
}

void clusteringStep::networkFunction()
{
	MPI_Request requests[workerSize];

/*
 * We only need the n-1 stages because the last stage is all singletons that
 * should just be concantenated to the end
 */
	for(std::size_t round=0; round<mergedFiles.size()-1; round++){
		ClusterSet cs(nr[round]);

		networkInit(round, cs);
		int filesRead = networkInit2(round, requests);

		for(std::size_t i=round+filesRead+1; i<mergedFiles.size(); i++ ){
			networkFunctionRound(requests, cs, i);
		}

		// We dont need the last round, all singletons
		// penultimate round
		gatherRemains(requests, ioargs->block_size, cs);
		if ( round == mergedFiles.size()-2 ) {
			terminateCluster(false);
		} else {
			terminateCluster(true);
		}

		cs.writeCluster(ioargs->clusteredFile);
		cs.destroy();
	}

	//std::cout << "NETWORK THREAD EXITING" << std::endl;

	// Cancel all outstanding requests. We are done at this point and we need to
	// terminate the MPI environment
	for (int i = 0; i < workerSize; i++) {
		int flag;
		MPI_Test(&requests[i], &flag, MPI_STATUS_IGNORE);;
		//std::cout << "FLAG : " << flag << std::endl;
		if (!flag) {
			MPI_Cancel(&requests[i]);
			MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
		}
	}

	pthread_exit(0);
}

void clusteringStep::networkFunctionRound(MPI_Request *requests,
                                          ClusterSet &cs,
                                          std::size_t round)
{

	int index = -1;
	MPI_Waitany(workerSize, requests, &index, MPI_STATUS_IGNORE);

	reinitializeIrecv(index, requests, ioargs->block_size, cs);
	sendMoreInput(index, ioargs->block_size, round);
}

// Need to keep around num_read
//unsigned long total_num_read; // need to put this at the top
void clusteringStep::sendRepresentatives(unsigned long num_read, ClusterSet &cs){

	// Broadcast the size of the buffer
	MPI_Bcast(&cs.numReps, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);

	// Broadcast the the index start
	MPI_Bcast(&total_num_read, 1, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);

	// Broadcast the buffer
	MPI_Bcast(cs.potentialReps, (int) num_read, MPI::BYTE, MASTER, MPI_COMM_WORLD);

	total_num_read += num_read;
}

int clusteringStep::setupInput(const std::vector<std::string> &filenames,
                               std::size_t round) {

	int numRead = 0;

	std::size_t whichFile = round + 1;
	for(int i = 0; i<workerSize; i++){
		if(whichFile + i == filenames.size()){
			break;
		}

		readSortedBlockIntoBuffer(bufs->inputBuffers[i],
		                          filenames[whichFile + i]);

		numRead++;
	}
	return numRead;
}

//!! Extremely inefficient function, need to refactor and optimize
void clusteringStep::readSortedBlockIntoBuffer(char* buffer,
                                               const std::string &filename)
{

	// Iterate through the file and only read in sequences
	// which have not yet been clustered
	std::string accum = "";
	std::ifstream f_str(filename.c_str());
	while (!f_str.eof()) {
		fasta::Record rc;
		//f_str >> rc;
		extractFull(f_str, rc);

		if (rc.name() != "") {
			unsigned long index = rc.name().rfind("\t");
			std::string s = rc.name().substr(index + 1, rc.name().size());
			std::istringstream sss(s);
			unsigned long status;
			sss >> status;

			if (!bufs->clusteredStatus[status]) {
				std::string wr = rc.name() +"\n" + rc.sequence() + "\n";
				accum += wr;
			}
		}
	}

	sprintf(buffer, "%s", accum.c_str());

	if(strlen(buffer) == 0){
		std::cout << "READ IN EMPTY BUFFER" << std::endl;
	}
}


void clusteringStep::initializeRecvs(MPI_Request *requests,
                                     unsigned block_size){
	for(int i=0; i<workerSize; i++){
		MPI_Irecv(bufs->outputBuffers[i], block_size * WRITER_MULT, MPI_CHAR,
		          i + 1, OUTPUT, MPI_COMM_WORLD, &requests[i]);
	}
}

void clusteringStep::reinitializeIrecv(int index,
                                       MPI_Request *requests,
                                       unsigned block_size,
                                       ClusterSet &cs)
{
	// Reinitialize the Irecv
	char* output = bufs->outputBuffers[index];
	cs.parseBuffer(output, bufs->clusteredStatus, total_num_read);
	MPI_Irecv(output, block_size * WRITER_MULT, MPI_CHAR, index + 1, OUTPUT,
	          MPI_COMM_WORLD, &requests[index]);
	recieved_output++;
	memset(output, 0, block_size * WRITER_MULT);
}

void clusteringStep::sendMoreInput(int index, unsigned block_size, std::size_t round)
{
	char *input = bufs->inputBuffers[index];
	readSortedBlockIntoBuffer(input, mergedFiles[round]);

	if(strlen(input) == 0){
		std::cout << "SENDING EMPTY BUFFER SEND_MORE_INPUT" << std::endl;
	}

	sendHelper(input, block_size, index);
}

void clusteringStep::sendHelper(char* input, unsigned block_size, int index)
{
	MPI_Send(input, block_size, MPI_CHAR, index+1,
	         INPUT, MPI_COMM_WORLD);
	memset(input, 0, block_size);
	sent_input++;
}

void clusteringStep::terminateCluster(bool rounds_remaining)
{
	// Cleanup of the algorithm run
	MPI_Request finalRequests[workerSize];
	char terminationBuffer[10];
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
}

void clusteringStep::initializeSends(unsigned block_size){
	for(int index=0; index<workerSize; index++){
		char *tmp = bufs->inputBuffers[index];

		if(strlen(tmp) == 0){
			//std::cout << "SENDING EMPTY BUFFER INIT" << std::endl;
			break;
		} else {
			sendHelper(tmp, block_size, index);
		}
	}
}

void clusteringStep::lastClusterRound(){
	ClusterSet cs(nr.back());

	// Read the final Database
	std::stringstream s;
	s << "block" << nr.size()-1;
	Lastdb db(s.str());
	db.readFormattedDatabase(databaseDirectoryName);

	// Align the final cluster
	Lastal al(db.get_const_dbs(), ioargs->n_args);
	repClustering(al, nr.size()-1, cs);

	// Write everything out
	cs.writeCluster(ioargs->clusteredFile);
	cs.destroy();
}

void clusteringStep::repClustering(Lastal &al,
                                   std::size_t stages,
                                   ClusterSet &cs){

	//!!
	std::cout << "STAGES : " << stages << std::endl;
	//bufs->printClusteredStatus();

	readSortedBlockIntoBuffer(bufs->repBufInput,
	                          mergedFiles[stages]);

	// Self clustering
	bool* reps = bufs->clusteredStatus + total_num_read;
	Representatives rep(reps, total_num_read, nr[stages]); // for Master
	al.lastAlign(bufs->repBufInput, bufs->repBufOutput,
	             stages, rep);

	//printf("%s", bufs->repBufOutput);

	//!!
	cs.parseBuffer(bufs->repBufOutput, bufs->clusteredStatus, total_num_read);

	//!!
	//bufs->printClusteredStatus();

	memset(bufs->repBufOutput, 0, ioargs->block_size * WRITER_MULT);
}

clusteringStep::clusteringStep(const Arguments &args,
                               int c_size,
                               int w_size,
                               const std::string &database_name,
                               const std::vector<std::string> & filenames,
                               const std::vector<std::size_t> & nspf) :
		clusterSize(c_size),
		workerSize(w_size),
		databaseDirectoryName(database_name),
		mergedFiles(filenames),
		nr(nspf),
		sent_input(0),
		recieved_output(0) {

	ioargs = new IOArguments(args, 1);
	bufs = new Buffers_Clustering(workerSize, ioargs->block_size, number_of_sequences);
}

clusteringStep::~clusteringStep() {
	delete ioargs;
	delete bufs;
}
