#include "LastdbArguments.hh"
#include "SubsetSuffixArray.hh"
#include "Alphabet.hh"
#include "MultiSequence.hh"
#include "io.hh"
#include "qualityScoreUtil.hh"
#include <numeric>  // accumulate

#include "lastdb.hh"
#include "mpi.h"

#define ERR(x) throw std::runtime_error(x)
#define LOG(x) if( args.verbosity > 0 ) std::cerr << "lastdb: " << x << '\n'

/* Lastdb::~Lastdb(){ } */


// Get the arguments for lastdb
Lastdb::Lastdb(const Arguments &args):length(5), num_files(7){

	// The file names which correspond to the current block partition
	db_files[0] = "block0.prj";
	db_files[1] = "block0.des";
	db_files[2] = "block0.bck";
	db_files[3] = "block0.sds";
	db_files[4] = "block0.ssp";
	db_files[5] = "block0.tis";
	db_files[6] = "block0.suf";
}

Lastdb::Lastdb(const std::string &database_file):length(5), num_files(7){

	// The file names which correspond to the current block partition
	db_files[0] = database_file + ".prj";
	db_files[1] = database_file + ".des";
	db_files[2] = database_file + ".bck";
	db_files[3] = database_file + ".sds";
	db_files[4] = database_file + ".ssp";
	db_files[5] = database_file + ".tis";
	db_files[6] = database_file + ".suf";
}

Lastdb::Lastdb():length(5), num_files(7){

	db_files[0] = "block0.prj";
	db_files[1] = "block0.des";
	db_files[2] = "block0.bck";
	db_files[3] = "block0.sds";
	db_files[4] = "block0.ssp";
	db_files[5] = "block0.tis";
	db_files[6] = "block0.suf";
}

void Lastdb::readFormattedDatabase(const std::string &directoryLastdb) {

	for(int i=0; i<num_files; i++) {
		const std::string file_name = db_files[i];
		const std::string fullpath = directoryLastdb + "/" + file_name;
		std::ifstream database_file;

		// Open the database file as long as it's not the prj or the des files
		if (file_name.find(".prj") != std::string::npos || file_name.find(".des") != std::string::npos) {
			database_file.open(fullpath.c_str());
		} else {
			database_file.open(fullpath.c_str(), std::ifstream::binary);
		}

		// Get the length of the file
		database_file.seekg(0, std::ios::end);
		long buffer_length = database_file.tellg();
		buffer_length++;

		// Read the file of the current partition into a buffer and save the name and length
		database_file.seekg(0, std::ios::beg);
		char *buf = new char[buffer_length];
		database_file.read(buf, buffer_length);
		formatted_dbs[file_name] = buf;
		db_lengths[file_name] = buffer_length;
	}
}

// Distribute the formatted block 0 files from process 0 to all other processes
void Lastdb::distributeFormattedDatabase(){

	for (int i=0; i<num_files; i++) {
		std::string file_name = db_files[i];
		// Broadcast the size of the buffer
		MPI_Bcast(&db_lengths[file_name], 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		// Broadcast the buffer
		MPI_Bcast(formatted_dbs[file_name], (int)db_lengths[file_name], MPI::BYTE, 0, MPI_COMM_WORLD);
	}
}

std::map<std::string, char*> Lastdb::get_dbs() const{
	return formatted_dbs;
};

std::map<std::string, char*>& Lastdb::get_dbs2(){
	return formatted_dbs;
};

std::map<std::string, long> Lastdb::get_db_lengths() const{
	return db_lengths;
}

std::map<std::string, long>& Lastdb::get_db_lengths2(){
	return db_lengths;
}

std::string Lastdb::get_db_files(int i) const{
	return db_files[i];
}

const std::map<std::string, char *> &Lastdb::get_const_dbs() const{
	return formatted_dbs;
};

void Lastdb::set_db_files(const std::string &name, int position){
	db_files[position] = name;
}

void Lastdb::set_db_length(long length, std::string file){
	db_lengths[file] = length;
}

//!! This creates buffers that we can avoid
// Recieve the formatted databases from the master
void Lastdb::recieveFormattedDatabase(){

	for (int i=0; i<num_files; i++) {
		std::string file_name = db_files[i];
		// Recieve the broadcast of the size of the buffer
		MPI_Bcast(&db_lengths[file_name], 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

		// Create a buffer to hold the formatted database s
		formatted_dbs[file_name] = new char[db_lengths[file_name]];

		// Recieve the broadcast of the buffer
		MPI_Bcast(formatted_dbs[file_name], (int)db_lengths[file_name],
		          MPI::BYTE, 0, MPI_COMM_WORLD);
	}
}

void Lastdb::writeDatabase(const std::string &directory, const std::string &base) {

	for(int i=0; i<num_files; i++) {
		std::string file_name = db_files[i];

		if (file_name.find(".prj") != std::string::npos || file_name.find(".des") != std::string::npos) {
			// Write out ASCII file
			//std::ofstream out((directory+"/"+file_name).c_str());
			std::ofstream out((directory+"/"+file_name).c_str());
			out.write(formatted_dbs[file_name], db_lengths[file_name]);
		} else {
			// Write out binary file
			//std::ofstream out((directory+"/"+file_name).c_str(), std::ios_base::binary);
			std::ofstream out((directory+"/"+file_name).c_str(), std::ios_base::binary);
			out.write(formatted_dbs[file_name], db_lengths[file_name]);
		}
	}
}


void Lastdb::clear(){
	std::map<std::string, char*>::iterator it;
	for(it=formatted_dbs.begin(); it!=formatted_dbs.end(); ++it) {
		if(it->second != NULL) {
			delete[] it->second;
			it->second = NULL;
		}
	}
}

using namespace cbrc;

typedef MultiSequence::indexT indexT;
typedef unsigned long long countT;

// Set up an alphabet (e.g. DNA or protein), based on the user options
void makeAlphabet( Alphabet& alph, const LastdbArguments& args ){
	if( !args.userAlphabet.empty() )  alph.fromString( args.userAlphabet );
	else if( args.isProtein )         alph.fromString( alph.protein );
	else                              alph.fromString( alph.dna );
}

// Does the first sequence look like it isn't really DNA?
bool isDubiousDna( const Alphabet& alph, const MultiSequence& multi ){
	const uchar* seq = multi.seqReader() + multi.seqBeg(0);
	unsigned dnaCount = 0;

	for( indexT i = 0; i < 100; ++i ){  // look at the first 100 letters
		uchar c = alph.canonical[ seq[i] ];
		if( c == alph.size ) return false;  // we hit the end of the sequence early
		if( c < alph.size || c == alph.encode[ (uchar)'N' ] ) ++dnaCount;
	}

	bool ret = dnaCount < 90;
	/*
  if( dnaCount < 90 ) ret = true;  // more than 10% unexpected letters
  else return false;
	 */
	return ret;
}

const unsigned maxNumOfIndexes = 16;

static void addSeeds( SubsetSuffixArray indexes[], unsigned& numOfIndexes,
                      const std::vector<std::string>& seedStrings,
                      const LastdbArguments& args, const Alphabet& alph ){
	for( unsigned x = 0; x < seedStrings.size(); ++x ){
		if( numOfIndexes >= maxNumOfIndexes ) ERR( "too many seed patterns" );
		CyclicSubsetSeed& seed = indexes[ numOfIndexes++ ].getSeed();
		seed.fromString( seedStrings[x], args.isCaseSensitive, alph.encode );
	}
}

// Set up the seed pattern(s), and return how many of them there are
unsigned makeSubsetSeeds( SubsetSuffixArray indexes[],
                          const LastdbArguments& args, const Alphabet& alph ){
	unsigned numOfIndexes = 0;
	const std::string& a = alph.letters;

	for( unsigned x = 0; x < args.subsetSeedFiles.size(); ++x ){
		const std::string& name = args.subsetSeedFiles[x];
		std::vector<std::string> s = CyclicSubsetSeed::fromName( name );
		addSeeds( indexes, numOfIndexes, s, args, alph );
	}

	for( unsigned x = 0; x < args.seedPatterns.size(); ++x ){
		const std::string& mask = args.seedPatterns[x];
		std::vector<std::string> s = CyclicSubsetSeed::fromMask( a, mask );
		addSeeds( indexes, numOfIndexes, s, args, alph );
	}

	if( numOfIndexes == 0 ){
		if( alph.letters == alph.dna ){
			const char* mask = "1T1001100101";  // YASS
			std::vector<std::string> s = CyclicSubsetSeed::fromMask( a, mask );
			addSeeds( indexes, numOfIndexes, s, args, alph );
		}
		else{
			std::vector<std::string> s = CyclicSubsetSeed::fromMask( a, "1" );
			addSeeds( indexes, numOfIndexes, s, args, alph );
		}
	}

	return numOfIndexes;
}

void writePrjFile( const std::string& fileName, const LastdbArguments& args,
                   const Alphabet& alph, countT sequenceCount,
                   const std::vector<countT>& letterCounts,
                   unsigned volumes, unsigned numOfIndexes,
                   std::map<std::string, char*> &dbs,
                   std::map<std::string, long> &db_lengths){

	countT letterTotal = std::accumulate( letterCounts.begin(),
	                                      letterCounts.end(), countT(0) );

	std::stringstream f;
	f << "version=" <<
	#include "version.hh"
	<< '\n';
	f << "alphabet=" << alph << '\n';
	f << "numofsequences=" << sequenceCount << '\n';
	f << "numofletters=" << letterTotal << '\n';
	f << "letterfreqs=";
	for( unsigned i = 0; i < letterCounts.size(); ++i ){
		if( i > 0 ) f << ' ';
		f << letterCounts[i];
	}
	f << '\n';

	if( !args.isCountsOnly ){
		f << "maxunsortedinterval=" << args.minSeedLimit << '\n';
		f << "masklowercase=" << args.isCaseSensitive << '\n';
		if( args.inputFormat != sequenceFormat::fasta ){
			f << "sequenceformat=" << args.inputFormat << '\n';
		}
		if( volumes+1 > 0 ){
			f << "volumes=" << volumes << '\n';
		} else {
			f << "numofindexes=" << numOfIndexes << '\n';
		}
	}

	db_lengths["block0.prj"] = f.str().size();
	dbs["block0.prj"] = new char[1000];
	sprintf(dbs["block0.prj"], f.str().c_str());
}

// Make one database volume, from one batch of sequences
void makeVolume( SubsetSuffixArray indexes[], unsigned numOfIndexes,
                 const MultiSequence& multi, const LastdbArguments& args,
                 const Alphabet& alph, const std::vector<countT>& letterCounts,
                 const std::string& baseName,
                 std::map<std::string, char*> &dbs,
                 std::map<std::string, long> &db_lengths){

	LOG( "writing..." );
	writePrjFile( baseName + ".prj", args, alph, multi.finishedSequences(),
	              letterCounts, -1, numOfIndexes, dbs, db_lengths);
	multi.toFiles( "block0", dbs, db_lengths );

	LOG( "sorting..." );
	indexes[0].sortIndex( multi.seqReader(), args.minSeedLimit );
	LOG( "bucketing..." );
	indexes[0].makeBuckets( multi.seqReader(), args.bucketDepth );
	LOG( "writing..." );
	indexT textLength = multi.finishedSize();
	indexes[0].toFiles( "block0", true, textLength, dbs, db_lengths );
	indexes[0].clearPositions();
	LOG( "done!" );
}

// The max number of sequence letters, such that the total volume size
// is likely to be less than volumeSize bytes.  (This is crude, it
// neglects memory for the sequence names, and the fact that
// lowercase-masked letters and DNA "N"s aren't indexed.)
static indexT maxLettersPerVolume( const LastdbArguments& args,
                                   unsigned numOfIndexes ){
	std::size_t bytesPerLetter = isFastq( args.inputFormat ) ? 2 : 1;
	std::size_t maxIndexBytesPerPosition = sizeof(indexT) + 1;
	maxIndexBytesPerPosition *= numOfIndexes;
	std::size_t x = bytesPerLetter * args.indexStep + maxIndexBytesPerPosition;
	std::size_t y = args.volumeSize / x * args.indexStep;
	indexT z = y;
	if( z < y ) z = indexT(-1);
	return z;
}

// Read the next sequence, adding it to the MultiSequence and the SuffixArray
std::size_t appendFromFasta( MultiSequence& multi,
                             SubsetSuffixArray indexes[],
                             unsigned numOfIndexes,
                             const LastdbArguments& args,
                             const Alphabet& alph,
                             char *& ptr,
                             std::size_t read_so_far,
                             std::size_t length){

	indexT maxSeqLen = maxLettersPerVolume( args, numOfIndexes );
	if( multi.finishedSequences() == 0 ) maxSeqLen = indexT(-1);

	std::size_t oldUnfinishedSize = multi.unfinishedSize();
	indexT oldFinishedSize = multi.finishedSize();

	std::size_t bytes_read = multi.appendFromFasta(ptr);

	/*
	std::cout << "READ_SO_FAR : " << read_so_far << std::endl;
	std::cout << "BYTES_READ : " << bytes_read << std::endl;
	std::cout << "STRLEN : " << length << std::endl;
	 */

	if( !multi.isFinished() && multi.finishedSequences() == 0 )
		ERR( "encountered a sequence that's too long" );

	// encode the newly-read sequence
	alph.tr( multi.seqWriter() + oldUnfinishedSize,
	         multi.seqWriter() + multi.unfinishedSize() );

	if( isPhred( args.inputFormat ) )  // assumes one quality code per letter:
		checkQualityCodes( multi.qualityReader() + oldUnfinishedSize,
		                   multi.qualityReader() + multi.unfinishedSize(),
		                   qualityOffset( args.inputFormat ) );

	read_so_far += bytes_read;

	if( read_so_far <= length && multi.isFinished() && !args.isCountsOnly ){
		//if( in && multi.isFinished() && !args.isCountsOnly ){
		for( unsigned x = 0; x < numOfIndexes; ++x ){
			indexes[x].addPositions( multi.seqReader(), oldFinishedSize,
			                         multi.finishedSize(), args.indexStep );
		}
	}
	return bytes_read;
}



void Lastdb::createLastDatabase(char* input_buffer,
                                std::map<std::string, char*> &dbs,
                                std::map<std::string, long> &db_lengths){

	LastdbArguments args;
	args.fromArgs();

	Alphabet alph;
	MultiSequence multi;
	SubsetSuffixArray indexes[maxNumOfIndexes];
	makeAlphabet( alph, args );
	unsigned numOfIndexes = makeSubsetSeeds( indexes, args, alph );
	multi.initForAppending(1);
	alph.tr( multi.seqWriter(), multi.seqWriter() + multi.unfinishedSize() );
	countT sequenceCount = 0;
	std::vector<countT> letterCounts( alph.size );

	char* ptr = input_buffer;
	std::size_t length = strlen(ptr);
	std::size_t read_so_far = 0;

	int count = 0;
	while(read_so_far <= length ){
		read_so_far += appendFromFasta( multi, indexes, numOfIndexes,
		                                args, alph, ptr, read_so_far, length);

		count++;

		if( !args.isProtein && args.userAlphabet.empty() &&
		    sequenceCount == 0 && isDubiousDna( alph, multi ) ){
			std::cerr << "lastdb: that's some funny-lookin DNA\n";
		}

		if( multi.isFinished() ){
			++sequenceCount;
			indexT lastSeq = multi.finishedSequences() - 1;
			alph.count( multi.seqReader() + multi.seqBeg(lastSeq),
			            multi.seqReader() + multi.seqEnd(lastSeq),
			            &letterCounts[0] );
			// memory-saving, which seems to be important on 32-bit systems:
			if( args.isCountsOnly ) multi.reinitForAppending();
		}
	}
	/*
	std::cout << "count : " << count << std::endl;
	std::cout << "read_so_far : " << read_so_far << std::endl;
	std::cout << "length : " << length << std::endl;
	 */

	makeVolume( indexes, numOfIndexes,
	            multi, args, alph, letterCounts,
	            args.lastdbName, dbs,
	            db_lengths);
}
