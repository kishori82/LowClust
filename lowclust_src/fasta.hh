#ifndef FASTA_HH
#define FASTA_HH 

// The base of this code was taken from https://github.com/tdoug870/fasta-parser
#include <string>
#include <set>

extern unsigned long long counter;

namespace fasta {

  class Record {

    std::string d_name;     
    std::string d_sequence;  
    int d_length;

    friend std::istream& operator>>(std::istream&, Record&);

    public:
    explicit Record();

    Record(const std::string& name, const std::string& sequence, const int &length);
    //Record(const Record& original);

    // Operators 
    Record& operator=(const Record& rhs);

    bool operator<(const Record& rec) const;
    bool operator>(const Record& rec) const;

    // Setters
    void set_name(const std::string& value);
    void set_sequence(const std::string& value);
    void set_length(const int &value);
    // Getters 
    const std::string& name() const;
    const std::string& sequence() const;
    const int length() const;

    //std::ostream& print(std::ostream& stream) const;
		  //std::ostream& print(std::ostream& stream, unsigned long &rank) const;
    std::ostream &print(std::ostream &stream, unsigned long rank) const;

		  unsigned long get_print_size(unsigned long &rank) const;
  };

  // FREE OPERATORS
	/*
  bool operator==(const Record& lhs, const Record& rhs);
  bool operator!=(const Record& lhs, const Record& rhs);
	 */

  std::istream& operator>>(std::istream& stream, Record& rhs);
  std::ostream& operator<<(std::ostream& stream, const Record& rhs);

	std::istream &extractFull(std::istream &stream, Record &rhs);

}  // close package namespace

#endif
