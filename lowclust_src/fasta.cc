#include <sstream>
#include <set>
#include <iostream> // <<
#include "fasta.hh"

unsigned long long counter;

namespace fasta {

  bool Record::operator<(const Record& rec) const
  {
    return d_length < rec.length();
  }

  bool Record::operator>(const Record& rec) const
  {
    return d_length > rec.length();
  }

  std::istream& operator>>(std::istream& stream, Record& rhs)
  {
    std::string line;
    if (getline(stream, line)) {
	    if ('>' == line[0]) {
		    std::string name = line;
		    std::size_t pos = name.find_first_of('\t');
		    std::size_t pos2 = name.find_first_of(' ');
		    if (pos < pos2) {
			    name = name.substr(0, pos);
		    } else {
			    name = name.substr(0, pos2);
		    }
		    rhs.set_name(name);

		    std::string sequence;
		    while (getline(stream, line) && '>' != stream.peek()) {
			    sequence += line;
		    }
		    if ('>'== stream.peek()){
			    sequence += line;
		    }

		    rhs.set_sequence(sequence);
		    rhs.set_length(sequence.length());
		    stream.clear();
	    } else {
		    stream.clear(std::ios_base::failbit);
	    }
    }
	  return stream;
  }

	std::istream &extractFull(std::istream &stream, Record &rhs) {
		std::string line;
		if (getline(stream, line)) {
			if ('>' == line[0]) {
				rhs.set_name(line);

				std::string sequence;
				while (getline(stream, line) && '>' != stream.peek()) {
					sequence += line;
				}
				if ('>' == stream.peek()) {
					sequence += line;
				}

				rhs.set_sequence(sequence);
				rhs.set_length(sequence.length());
				stream.clear();
			} else {
				stream.clear(std::ios_base::failbit);
			}
		}
		return stream;
	}

	std::ostream& operator<<(std::ostream& stream, const Record& rhs)
	{
		stream << "[ " << rhs.name() << ", " << rhs.sequence() << ", " << rhs.length() << " ]";
		return stream;
	}

	std::ostream &Record::print(std::ostream &stream, unsigned long rank) const
	{
		if (stream.good()) {
			stream << d_name << "\t" << d_length << "\t" << rank << "\n" << d_sequence << "\n";
		}
		return stream;
	}

	unsigned long Record::get_print_size(unsigned long &rank) const {
		std::stringstream s;
		s << d_name << "\t" << d_length << "\t" << rank << "\n" << d_sequence << "\n";
		return s.str().size();
	}

	// CREATORS

	Record::Record()
			: d_name()
			, d_sequence()
			, d_length()
	{}

	Record::Record(const std::string& name, const std::string& sequence, const int &length)
			: d_name(name)
			, d_sequence(sequence)
			, d_length(length)
	{}


  // MANIPULATORS

  Record& Record::operator=(const Record& rhs){
    d_name     = rhs.d_name;
    d_sequence = rhs.d_sequence;
    d_length 	 = rhs.d_length;

    return *this;
  }


  void Record::set_name(const std::string& value){
    d_name.assign(value.begin(), value.end());
  }


  void Record::set_sequence(const std::string& value){
    d_sequence.assign(value.begin(), value.end());
  }


  void Record::set_length(const int& value){
    d_length = value;
  }

  // ACCESSORS

  const std::string& Record::name() const{
    return d_name;
  }


  const std::string& Record::sequence() const{
    return d_sequence;
  }


  const int Record::length() const{
    return d_length;
  }

}  // close package namespace
