#ifndef LASTDB_HH
#define LASTDB_HH

#include <arguments.hh>
#include <map>
#include <string>
#include "tempfiles.hh"

class Lastdb{

  private:
    std::size_t length;

    std::map<std::string, char*> formatted_dbs;
    std::map<std::string, long> db_lengths;
    std::string db_files[7];
    int num_files;

  public:
    Lastdb(const Arguments &args);
		Lastdb(const std::string &database_file);
		Lastdb();
		//~Lastdb();

		// Master methods
		void createLastDatabase(char* input_buffer,
		                        std::map<std::string, char*> &dbs,
		                        std::map<std::string, long> &db_lengths);
    void readFormattedDatabase(const std::string &directoryLastdb);
    void distributeFormattedDatabase();

		void writeDatabase(const std::string &directory, const std::string &base);
		void clear();

    // Worker methods
    void recieveFormattedDatabase();

		// Getters
    std::map<std::string, char*> get_dbs() const;
		const std::map<std::string, char *> &get_const_dbs() const;
		std::map<std::string, char *>& get_dbs2();
		std::map<std::string, long> get_db_lengths() const;
		std::map<std::string, long>& get_db_lengths2();
		std::string get_db_files(int i) const;

		// Setters
		void set_db_files(const std::string &name, int position);
		void set_db_length(long length, std::string file);

};

void lastdb( int argc, char** argv, char* input_buffer,
             std::map<std::string, char*> &dbs,
             std::map<std::string, long> &db_lengths);

#endif
