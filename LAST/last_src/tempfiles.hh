#ifndef TEMPFILES_HH
#define TEMPFILES_HH

#include <iostream>
#include <map>
#include <stack>
#include <stdlib.h>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <cstring>

class TempFiles {
private:
		std::string filename(unsigned int i);
		std::string dirname(unsigned int i);
		std::string toString(unsigned int i);

		std::string tempdir;
		std::string basedir ;
		std::string base;
		std::string suffix;
		std::string path;

		void remove_dir(char *path);
		std::vector<std::string> filenames;

		unsigned int count;
		unsigned int S; // fanout

public:
		TempFiles(const std::string &_tempdir,
		          const std::string &_basedir,
		          const std::string &basename,
		          const std::string &suffixname = "");

		//void setFanOut(unsigned int i ) {  S = i; }

		std::string nextFileName();
		//std::string nextFileName(int rank);
		void clear();
		unsigned long size();

		const std::vector<std::string>& getFileNames() const;
		//const std::vector<std::string> getOrderedFileNames() ;

		void generateDirectory();

		//std::string get_path();
};

#endif
