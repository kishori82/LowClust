#include "arguments.hh"
#include <getopt.h> // getopt
#include <sys/stat.h> // stat
#include <cstdlib> // atoi, atof
#include <iostream>

void Arguments::parseOptions(int argc, char **argv) {
	const char *optString = "i:o:s:b:d:v::h::";
  int opt = getopt(argc, argv, optString);
  while (opt != -1) {
    switch (opt) {
      case 'i':
        input = optarg;
        break;
      case 'o':
        output_dir = optarg;
        break;
	    case 'd':
		    tmpDir = optarg;
		    break;
      case 's':
        similarity = atoi(optarg);
		    similarity = similarity/100;
        break;
      case 'b':
	      block_mb = (unsigned) atoi(optarg);
        break;
	    case 'p':
		    numThreads = (unsigned) atoi(optarg);
		    break;
      case 'v':
        verbosity = true;
        break;
      case 'h':
        help = true;
        break;
	    default:
		    std::cerr << "Unrecongized option" << std::endl;
    }
    opt = getopt(argc, argv, optString);
  }
}

void Arguments::printHelp(){
  if(help){
    std::cout << "Minimal Usage Example: \n"
      << "\tmpirun -np cores ./LowClust -i input -o output_directory -s similarity -b block_mb\n"
      << "\tmpirun -np 10 ./LowClust -i input.fna -o output_dir -s 30 -b 1" <<
		    std::endl;
	  std::cout << "The arguments for LowClust are as follows: \n"
	  << "options which require arguments:\n"
	  << "\t-i input file\n"
	  << "\t-o output directory\n"
	  << "\t-d directory for temporary files\n"
	  << "\t-s similarity threshold\n"
	  << "\t-b block size\n"
	  << "\t-p number of cores per MPI node\n"
	  << "options which do not require arguments:\n"
	  << "\t-v verbose mode\n"
	  << "\t-h help" << std::endl;
    throw(-1);
  }
}

void Arguments::checkRequiredArguments(){
  if(input == ""){
    std::cout << "LowClust requires an input file argument for the -i flag" << std::endl;
	  std::cout << "EXAMPLE input file : -i input_file.fna" << std::endl;
    throw(-1);
  }else{
    struct stat st; 
    if(stat(input.c_str(), &st) == -1 ) { 
      std::cout << "The input file specified by -i does not exist" << std::endl;
      throw(-1);
    }
  }
  if(output_dir == ""){
    std::cout << "LowClust requires an output directory argument for the -o flag" << std::endl;
	  std::cout << "EXAMPLE output directory : -o output_dir" << std::endl;
    throw(-1);
  }
	if(similarity== 0){
		std::cout << "LowClust requires a similarity argument for the -s flag" << std::endl;
		std::cout << "EXAMPLE 30% similarity : -s 30" << std::endl;
		throw(-1);
	}
}

void Arguments::createDirectories(){
  // if the output directory does not exist, create it
  struct stat sb;
  if (stat(output_dir.c_str(), &sb)) {
    mkdir(output_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }

	// if the tmp directory does not exist, create it
	struct stat sb2;
	if (stat(tmpDir.c_str(), &sb2)) {
		mkdir(tmpDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
}

// Check the output adn the tmpdir. if it doesnt exist try to create it

Arguments::Arguments(int argc, char **argv){
  input = "";
  output_dir = "";
	tmpDir = "/tmp";
	similarity = 0;
  block_mb = 25;
	numThreads = 1;
  verbosity = false;
  help = false; 

  try{
    parseOptions(argc, argv);
    printHelp();
    checkRequiredArguments();
    createDirectories();
  }catch(...){
    throw(-1);
  }
}

std::string Arguments::get_input() const{
  return input; 
}

std::string Arguments::get_output_dir() const{
  return output_dir;
}

std::string Arguments::get_tmp_dir() const {
	return tmpDir;
}

float Arguments::get_block_mb() const{
  return block_mb;
}


unsigned Arguments::get_num_threads() const {
	return numThreads;
}

/*
bool Arguments::get_verbosity() const{
	return verbosity;
}
*/

float Arguments::get_similarity() const{
	return similarity;
}
