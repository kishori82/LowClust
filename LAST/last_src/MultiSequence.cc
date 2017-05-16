// Copyright 2008, 2009, 2010, 2011 Martin C. Frith

#include "MultiSequence.hh"
#include "io.hh"
#include <iterator>  // istreambuf_iterator

using namespace cbrc;

void MultiSequence::initForAppending( indexT padSizeIn ){
  padSize = padSizeIn;
  seq.v.assign( padSize, ' ' );
  ends.v.assign( 1, padSize );
  names.v.clear();
  nameEnds.v.assign( 1, 0 );
}

void MultiSequence::reinitForAppending(){
  seq.v.erase( seq.v.begin(), seq.v.begin() + ends.v.back() - padSize );
  names.v.erase( names.v.begin(),
      names.v.begin() + nameEnds.v[ finishedSequences() ] );
  ends.v.resize(1);
  nameEnds.v.resize(1);
  if( !names.v.empty() ) nameEnds.v.push_back( names.v.size() );
}

void MultiSequence::fromMemory(const std::map<std::string, char *> &db_buffers,
                               indexT seqCount,
		//                          std::size_t qualitiesPerLetter,
		                           std::size_t round) {


	std::stringstream s;
	s << round;
	std::string block = "block" + s.str();
	ends.m.open(db_buffers.find(block+".ssp")->second, seqCount + 1);
	seq.m.open(db_buffers.find(block+".tis")->second, ends.m.back());
	nameEnds.m.open(db_buffers.find(block+".sds")->second, seqCount + 1);
	names.m.open(db_buffers.find(block+".des")->second, nameEnds.m.back());
	padSize = ends.m[0];
}

void MultiSequence::closeFiles(){
  ends.m.close(); 
  seq.m.close();
  nameEnds.m.close(); 
  names.m.close();
  qualityScores.m.close();
}

void MultiSequence::toFiles( const std::string& baseName,
                             std::map<std::string, char*> &dbs,
                             std::map<std::string, long> &db_lengths) const{

	memoryToBinaryFile( ends.begin(), ends.end(),
	                    baseName + ".ssp", dbs[baseName+".ssp"] );
	db_lengths[baseName+".ssp"] = (ends.end() - ends.begin())*sizeof(indexT);

	memoryToBinaryFile( seq.begin(), seq.begin() + ends.back(),
	                    baseName + ".tis", dbs[baseName+".tis"] );
	db_lengths[baseName+".tis"] = (seq.end() - seq.begin())*sizeof(uchar);

	memoryToBinaryFile( nameEnds.begin(), nameEnds.begin() + ends.size(),
	                    baseName + ".sds", dbs[baseName+".sds"] );
	db_lengths[baseName+".sds"] = (nameEnds.end() - nameEnds.begin())*sizeof(indexT);

	memoryToBinaryFile( names.begin(),
	                    names.begin() + nameEnds[ finishedSequences() ],
	                    baseName + ".des", dbs[baseName+".des"] );
	db_lengths[baseName+".des"] = (names.end() - names.begin())*sizeof(char);
}

void MultiSequence::addName( const std::string& name ){
	names.v.insert( names.v.end(), name.begin(), name.end() );
	nameEnds.v.push_back( names.v.size() );
  if( nameEnds.v.back() < names.v.size() )
    throw std::runtime_error("the sequence names are too long");
}

std::size_t MultiSequence::readFastaName(char* query_block) {

	// Find the first instance of the tab or newline character
	/*
	char c = 0;
	std::size_t i = 0;
	std::size_t length = 0;
	std::string name = "";
	while (c != '\t' && c != '\n'){
		c = query_block[i];
		length++;
		name += c;
		i++;
	}

	// continue until we hit the newline
	while ( c != '\n'){
		c = query_block[i];
		length++;
		i++;
	}
	*/

	//!! changed it to conserve our previous tabs used to delimit stuff
	char c = 0;
	std::size_t i = 0;
	std::size_t length = 0;
	std::string name = "";
	while (c != '\n'){
		c = query_block[i];
		length++;
		name += c;
		i++;
	}

	addName(name);
	//std::cout << name << std::endl;
	return length;
}


std::size_t MultiSequence::appendFromFasta(char *&query_block) {

	std::size_t nameSize = readFastaName(query_block);
	query_block += nameSize;

	std::size_t sequenceSize = 0;
	char c = 0;
	std::size_t i = 0;
	while ( c != '\n' ) {
		c = query_block[i];
		if (c == '>') break;  // we have hit the next FASTA sequence
		if (std::isspace(c) == 0) {
			seq.v.push_back(c);
		}
		i++;
		sequenceSize++;
	}
	sequenceSize++;

	finish();
	query_block += sequenceSize;
	return sequenceSize + nameSize;
}

/*
std::istream& MultiSequence::appendFromFasta( std::istream& stream, indexT maxSeqLen ){
  if( isFinished() ){
    char c = '>';
    stream >> c;
    if( c != '>' )
      throw std::runtime_error("bad FASTA sequence data: missing '>'");
    readFastaName(stream);
    if( !stream ) return stream;
  }

  std::istreambuf_iterator<char> inpos(stream);
  std::istreambuf_iterator<char> endpos;
  while( inpos != endpos ){
    uchar c = *inpos;
    if( c == '>' ) break;  // we have hit the next FASTA sequence
    if( !std::isspace(c) ){
      seq.v.push_back(c);
    }
    ++inpos;
  }

  finish();
  return stream;
}
 */

/*
// Keep the -1 is infinity scenario.
std::size_t MultiSequence::appendFromFastaLASTDB( char*& ptr, indexT maxSeqLen){

	std::size_t num_bytes_read = 0;
	std::size_t nameSize = 0;

	std::stringstream stream(ptr);

  if( isFinished() ){
    char c = '>';
    stream >> c;
	  num_bytes_read++;
    if( c != '>' ) {
	    throw std::runtime_error("bad FASTA sequence data: missing '>'");
    }
    nameSize = readFastaName(stream);
    if( !stream ) return false;
  }

  while( stream.good()){
	  char c;
	  stream >> c;
	  num_bytes_read++;
	  if( c == '>' ) break;  // we have hit the next FASTA sequence
	  if( !std::isspace(c) ){
		  if( seq.v.size() >= maxSeqLen ) break;
		  seq.v.push_back(c);
	  }
  }

	if( isFinishable(maxSeqLen) ) {
		finish();
	}

	num_bytes_read+=nameSize;
	ptr += num_bytes_read;
  return num_bytes_read;
}
 */

void MultiSequence::finish(){
  assert( !isFinished() );
  seq.v.insert( seq.v.end(), padSize, ' ' );
  ends.v.push_back( seq.v.size() );
  assert( ends.v.back() == seq.v.size() );
}

/*
void MultiSequence::unfinish(){
  assert( isFinished() );
  ends.v.pop_back();
  seq.v.erase( seq.v.end() - padSize, seq.v.end() );
}
 */

bool MultiSequence::isFinishable( indexT maxSeqLen ) const{
  return seq.v.size() + padSize <= maxSeqLen;
}

MultiSequence::indexT MultiSequence::whichSequence( indexT coordinate ) const{

  const indexT* u = std::upper_bound( ends.begin(), ends.end(), coordinate );
  assert( u != ends.begin() && u != ends.end() );
  return u - ends.begin() - 1;
}

std::string MultiSequence::seqName( indexT seqNum ) const{
  return std::string( names.begin() + nameEnds[ seqNum ],
      names.begin() + nameEnds[ seqNum + 1 ] );
}
