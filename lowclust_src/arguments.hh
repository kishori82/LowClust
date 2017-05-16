#ifndef ARGUEMENTS_HH
#define ARGUEMENTS_HH

#include <string>

class Arguments{
  private:
    std::string input; // -i
    std::string output_dir; // -o
		std::string tmpDir; // -d
    float similarity; // -s
    unsigned block_mb; // -b
		unsigned numThreads; // -p
    bool verbosity; // -v
    bool help; // -h

    void parseOptions(int argc, char **argv);
    void printHelp();
    void checkRequiredArguments();
    void createDirectories();

  public:
    Arguments(int argc, char **argv);

    std::string get_input() const;
    std::string get_output_dir() const;

		std::string get_tmp_dir() const;

		//bool get_verbosity() const;
    float get_block_mb() const;

		unsigned get_num_threads() const;
		float get_similarity() const;
};

#endif
