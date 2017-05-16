#include <iostream>
#include "arguments.hh"
#include "lowclust.hh"
#include "stages_master/sortingStep.h"
#include "stages_master/suffixArraysStep.h"
#include "stages_master/clusteringStep.h"

int pid;
int clusterSize;

// Stage deliverables

//SORTING STAGE
std::vector<std::string> merged_Files;
unsigned long number_of_sequences;

// SUFFIX ARRAYS STAGE
std::string database_Directory_Name;

// Stage directory structure
//std::vector<std::string> directories;

void mpiSetup(int argc, char **argv) {
	//!! I wonder if the init_thread is necessary
	// If it is necessary, we need to make it so that the main thread becomes the network thread
	//MPI_Init(&argc, &argv);
	int provided = 0;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
	/*
	if(provided == MPI_THREAD_SINGLE) { std::cout << "MPI_THREAD_SINGLE" << std::endl; }
	if(provided == MPI_THREAD_FUNNELED) { std::cout << "MPI_THREAD_FUNNELED" << std::endl; }
	if(provided == MPI_THREAD_SERIALIZED) { std::cout << "MPI_THREAD_SERIALIZED" << std::endl; }
	if(provided == MPI_THREAD_MULTIPLE) { std::cout << "MPI_THREAD_MULTIPLE" << std::endl; }
	 */

  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  MPI_Comm_size(MPI_COMM_WORLD, &clusterSize);
}

bool checkCluster(){
  if(clusterSize == 1){
    std::cerr << "The cluster must have at least one worker in it" << std::endl;
    MPI_Finalize();
    return false;
  }
  return true;
}

// Using MPI, sort everything in a parallel fashion
void sortFasta(const Arguments &args){

	if (pid==0) { // master

		std::cout << "Sorting the fasta" << std::endl;

		sortingStep sort(args, clusterSize, clusterSize-1);
		sort.runStep();
		merged_Files = sort.get_mergedFiles();
		//directories.push_back(sort.get_directory());

		std::cout << "Finished sorting the fasta\n\n\n" << std::endl;

	} else { // workers
		sortingStepWorker work(args);
		work.workerStage(args);
	}
}

void distributedSuffixArrays(const Arguments &args){

	if(pid==0) { // master

		std::cout << "Constructing suffix arrays" << std::endl;

		suffixArraysStep sa(args, clusterSize, clusterSize-1, merged_Files);
		sa.runStep();
		database_Directory_Name = sa.get_databaseDirectoryName();
		//directories.push_back(sa.get_directory());

		std::cout << "Finished constructing suffix arrays\n\n\n" << std::endl;

	} else { // workers
		suffixArraysStepWorker work(args);
		work.workerStage(args);
	}
}

void cluster(const Arguments &args){

  if(pid==0){ // master

	  std::cout << "Clustering sequences" << std::endl;

	  clusteringStep clust(args, clusterSize, clusterSize-1,
	                       database_Directory_Name, merged_Files,
	                       numSequencesPerFile); // from the extern

	  clust.runStep();
	  clust.lastClusterRound();
	  //clust.bufs->printClusteredStatus();

	  std::cout << "Finished clustering sequences\n\n\n" << std::endl;

  }else{ // workers
	  clusteringStepWorker work(args);
	  work.workerStage(args);
  }
}

int main(int argc, char**argv){
  try{
    mpiSetup(argc, argv);
    Arguments args(argc, argv);
    if(checkCluster()){
	    // Following functions use the MPI framework
	    sortFasta(args);
	    distributedSuffixArrays(args);
	    cluster(args);
	    MPI_Finalize();

	    /*
			removeDirectories(directories);
	     */

    }

  } catch ( const std::exception &ex ) {
	  std::cerr << ex.what() << std::endl;
	  //removeDirectories(directories);
    return -1;
  }

  if(pid==0){
    std::cout << "LowClust program has completed" << std::endl;
  }
}

void removeDirectories(const std::vector<std::string> &dirs){
	for(int i=0; i<dirs.size(); i++){
		// remove dirs[i].clear();
	}
}
