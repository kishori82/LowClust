//
// Created by david on 20/12/15.
//

#ifndef LC_CLUSTERINGSTEP_H
#define LC_CLUSTERINGSTEP_H

#include "stages_workers/clusteringStepWorker.h"
#include "semaphores/semaphores_clustering.h"
#include "semaphore_macros.h"
#include "ioArguments.h"
#include "stepInterface.h"
#include "Cluster.h"
#include "buffers/Buffers.h"
#include "buffers/Buffers_Clustering.h"

class clusteringStep : public stepInterface {

public:
		// POD objects
		Buffers_Clustering *bufs;
private:

		int clusterSize;
		int workerSize;

		int sent_input;
		int recieved_output;

// Vector of files after they have been merged and sorted
		std::vector<std::string> mergedFiles;
		std::string databaseDirectoryName;
		std::string clustersDirectoryName;

		IOArguments *ioargs;

		// number of represetnatives per file
		std::vector<std::size_t> nr;

		void readerFunction();
		void writerFunction();
		void networkFunction();
		void networkInit(std::size_t round, ClusterSet &cs);
		int networkInit2(std::size_t round, MPI_Request *requests);
		void networkFunctionRound(MPI_Request *requests,
		                          ClusterSet &cs,
		                          std::size_t round);

		int setupInput(const std::vector<std::string> &filenames,
		               std::size_t round);

		void readSortedBlockIntoBuffer(char* buffer,
		                               const std::string &filename);

		void sendRepresentatives(unsigned long read, ClusterSet &cs);

		void terminateCluster(bool rounds_remaining);
		void gatherRemains(MPI_Request *requests,
		                   unsigned block_size,
		                   ClusterSet &cs);

		void initializeRecvs(MPI_Request *requests,
		                     unsigned block_size);
		void reinitializeIrecv(int index,
		                       MPI_Request *requests,
		                       unsigned block_size,
		                       ClusterSet &cs);
		void sendMoreInput(int index,
		                   unsigned block_size,
		                   std::size_t round);
		void initializeSends(unsigned block_size);

		void sendHelper(char* input, unsigned block_size, int index);
		void repClustering(Lastal &al,
		                   std::size_t stages,
		                   ClusterSet &cs);

public:
		clusteringStep(const Arguments &args,
		               int c_size,
		               int w_size,
		               const std::string &database_name,
		               const std::vector<std::string> & filenames,
		               const std::vector<std::size_t> & nspf);

		~clusteringStep();

		void lastClusterRound();

		TempFiles get_directory();
};

#endif //LC_CLUSTERINGSTEP_H
