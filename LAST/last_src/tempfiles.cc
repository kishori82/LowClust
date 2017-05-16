#include <iostream>
#include <stack>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <sstream>
#include "tempfiles.hh"


TempFiles::TempFiles(const std::string &_tempdir,
          const std::string &_basedir,
          const std::string &basename,
          const std::string &suffixname) :
		tempdir(_tempdir),
		basedir(_basedir),
		base(basename),
		suffix(suffixname),
		count(0),
		//S(10), //!! this is problematic
		S(1000),
		path(this->tempdir + "/" + this->basedir) {}

std::string TempFiles::nextFileName() {
  std::stack<unsigned int>  names;
  std::string fullpath(this->tempdir + "/" + this->basedir);

  unsigned int i = this->count;
  unsigned int B = this->S;
  unsigned int r, f, d;

  r = i % B; 
  f = r;
  i = (i - r)/B;


  while( i > 0) {
    r = i % B; 
    d =  r-1 >=0 ? r-1 : B-1;
    names.push(d);
    i = (i - r)/B;
  }

  struct stat st;
  if(stat(fullpath.c_str(), &st)!=0 ) {
    mkdir(fullpath.c_str(), 0777);
  }

  while(!names.empty()) {
    fullpath = fullpath + std::string("/") + dirname(names.top());

    if(stat(fullpath.c_str(), &st)!=0 ) {
      mkdir(fullpath.c_str(), 0777);
    }
    names.pop();
  }

  fullpath = fullpath + std::string("/") + filename(f);

  this->filenames.push_back(fullpath);

  this->count++;


  return fullpath;
}

void TempFiles::generateDirectory(){
	std::string fullpath(this->tempdir + "/" + this->basedir);

	struct stat st;
	if(stat(fullpath.c_str(), &st)!=0 ) {
		mkdir(fullpath.c_str(), 0777);
	}
}

unsigned long TempFiles::size() {
  return filenames.size();
}

std::string TempFiles::filename(unsigned int i)  {
	return base+toString(i)+suffix;
}

std::string TempFiles::dirname(unsigned int i)  {
  return std::string("dir") + toString(i);
}

std::string TempFiles::toString(unsigned int i ) {
  char buf[100];
  sprintf(buf, "%d",i);
  return std::string(buf);
}


const std::vector<std::string>& TempFiles::getFileNames() const{
  return this->filenames;
}

void TempFiles::clear() {
  char path[500];
  std::string fullpath(this->tempdir + "/" + this->basedir);
  strcpy(path, fullpath.c_str());
  remove_dir(path);
  this->count =0 ;
  this->filenames.clear();
}

void TempFiles::remove_dir(char *path) {
  struct dirent *entry = NULL;
  DIR *dir = NULL;

  dir = opendir(path);

  if( dir ==0) return;
  while(entry = readdir(dir))
  {   
    DIR *sub_dir = NULL;
    FILE *file = NULL;
    char abs_path[200] = {0};
    if( strcmp(entry->d_name, ".")  && strcmp(entry->d_name, ".."))
    {   
      sprintf(abs_path, "%s/%s", path, entry->d_name);

      if(sub_dir = opendir(abs_path)) //if it is a directory
      {   
        closedir(sub_dir);
        remove_dir(abs_path);
      }   
      else 
      {   
        if(file = fopen(abs_path, "r"))
        {   
          fclose(file);
          remove(abs_path);
        }   
        else {
          remove(abs_path);

        }
      }   
    }   
  }   
  closedir(dir);
  remove(path);
}
