// Copyright 2008, 2009, 2010, 2011, 2012, 2013, 2014 Martin C. Frith
// BLAST-like pair-wise sequence alignment, using suffix arrays.
#include "lastal.hh"
#include "lastThreads.h"

threadData **threadDatas;
pthread_t writerThread;

std::queue<int> idInputQueue;
std::queue<MultiSequence *> inputQueue;
std::queue<int> idOutputQueue;
std::queue<std::vector<Match*> *> outputQueue;

semaphores_last *sems;

bool roundDone = false;
bool threadTerminate = false;

unsigned volumes = 1;
int readSequences = 0;
int doneSequences = 0;

LastalArguments args;

Alphabet *queryAlph;
Alphabet *alph;
MultiSequence *text;
ScoreMatrix *scoreMatrix;
TwoQualityScoreMatrix *twoQualityScoreMatrix;
TwoQualityScoreMatrix *twoQualityScoreMatrixMasked;
GeneralizedAffineGapCosts gapCosts;
OneQualityScoreMatrix *oneQualityScoreMatrix;

SubsetSuffixArray suffixArray;

sequenceFormat::Enum referenceFormat = sequenceFormat::fasta;
unsigned numOfIndexes; // assume this value, if unspecified
int minScoreGapless;
float sim_cutoff;

std::size_t r;

void createStructures(const std::map<std::string, char *> &dbs,
                      indexT &bucketDepth,
                      indexT &seqCount,
                      indexT &delimiterNum,
                      indexT &seqLen,
                      std::size_t round) {

	alph = new Alphabet();
	scoreMatrix = new ScoreMatrix();
	sems = new semaphores_last();
	text = new MultiSequence();
	oneQualityScoreMatrix = new OneQualityScoreMatrix();
	twoQualityScoreMatrixMasked = new TwoQualityScoreMatrix();
	twoQualityScoreMatrix = new TwoQualityScoreMatrix();

	readOuterPrj(volumes, seqCount, seqLen, delimiterNum, bucketDepth, dbs, round);

	bool isMultiVolume = volumes > 1;

	args.setDefaultsFromAlphabet(alph->letters == alph->dna, alph->isProtein(),
	                              isCaseSensitiveSeeds, isMultiVolume);

	makeScoreMatrix();

	gapCosts.assign(args.gapExistCost, args.gapExtendCost,
	                args.insExistCost, args.insExtendCost, args.gapPairCost);
	minScoreGapless = args.calcMinScoreGapless(seqLen, numOfIndexes);
	if (!isMultiVolume) {
		args.minScoreGapless = minScoreGapless;
	}
	queryAlph = alph;
}

void makeScoreMatrix(){

	const char *b = args.isTranslated() ? "BLOSUM80" : "BLOSUM62";
	scoreMatrix->fromString(ScoreMatrix::stringFromName(b));
	scoreMatrix->init(alph->encode);
}

void readOuterPrj(unsigned &volumes,
                  indexT &seqCount,
                  indexT &seqLen,
                  indexT &delimiterNum,
                  indexT &bucketDepth,
                  const std::map<std::string, char *> &dbs,
                  std::size_t round) {

	std::stringstream s;
	s << round;
	std::string filename = "block"+s.str()+".prj";

	std::stringstream f(dbs.find(filename)->second);

	unsigned version = 0;

	std::string line, word;
	while (getline(f, line)) {
		std::istringstream iss(line);
		getline(iss, word, '=');
		if (word == "version") iss >> version;
		if (word == "alphabet") iss >> (*alph);
		if (word == "numofsequences") iss >> seqCount;
		if (word == "numofletters") iss >> seqLen;
		if (word == "maxunsortedinterval") iss >> minSeedLimit;
		if (word == "masklowercase") iss >> isCaseSensitiveSeeds;
		if (word == "sequenceformat") iss >> referenceFormat;
		if (word == "volumes") iss >> volumes;
		if (word == "numofindexes") iss >> numOfIndexes;
		if (word == "specialcharacters") iss >> delimiterNum;
		if (word == "prefixlength") iss >> bucketDepth;
		if (word == "subsetseed")
			suffixArray.seed.appendPosition(iss,
			                                isCaseSensitiveSeeds,
			                                alph->encode);
	}

	if (alph->letters.empty() || seqCount + 1 == 1 || seqLen + 1 == 1 ||
	    isCaseSensitiveSeeds < 0 || referenceFormat >= sequenceFormat::prb) {
		f.setstate(std::ios::failbit);
	}
}

void readIndex(indexT bucketDepth,
               indexT seqCount,
               indexT delimiterNum,
               const std::map<std::string, char *> &dbs,
               std::size_t round) {

	text->fromMemory(dbs, seqCount, /*isFastq( referenceFormat ),*/ round);

	suffixArray.fromMemory(dbs, text->unfinishedSize() - delimiterNum,
	                       bucketDepth, round);
}

void initializeWorkers(const Representatives &rep) {

	threadDatas = new threadData *[args.threadNum];

	for (int i = 0; i < args.threadNum; i++) {
		threadDatas[i] = new threadData(i, rep);
		threadDatas[i]->startThread();

		for (int k = 0; k < 2; k++) {
			SEM_POST(threadDatas[i]->tsems.readSema);
		}
	}
}

void *__writerFunction(void *arguments) {

	char* outputBuffer = (char*)arguments;

	int id;
	std::vector<Match*> *current;

	while (true) {
		SEM_WAIT(sems->writerSema);
		SEM_WAIT(sems->inputOutputQueueSema);
		current = outputQueue.front();
		outputQueue.pop();
		id = idOutputQueue.front();
		idOutputQueue.pop();
		SEM_POST(sems->inputOutputQueueSema);

		threadData *data = threadDatas[id];

		//!! strcat is a poor choice...
		//!! Remove the giveFields functionality with a special strcat function
		//!! Fix the functionality of the Match::strcat function and remove the others
		unsigned long outputBufferLength = 0;
		for (std::size_t j = 0; j < current->size(); j++) {
			if (!current->at(j)->dead) { // check if it's viable
				//strcat(outputBuffer, current->at(j)->giveFields().c_str());

				std::string result = current->at(j)->giveFields();
				faststrcatString(outputBuffer, outputBufferLength, result);
				outputBufferLength += result.size();

				//outputBufferLength += Match::strcat(outputBuffer, outputBufferLength, current->at(j));
			}
		}

		//current->clear();
		for(std::size_t j=0; j<current->size(); j++){
			delete current->at(j);
		}

		SEM_WAIT(sems->inputOutputQueueSema);
		data->outputVectorQueue.push(current);
		doneSequences++;
		SEM_POST(sems->inputOutputQueueSema);

		SEM_POST(data->tsems.writeSema);

		SEM_WAIT(sems->roundCheckSema);
		if (roundDone && readSequences == doneSequences) {
			SEM_POST(sems->terminationSema);
			SEM_POST(sems->roundCheckSema);
			break;
		}
		SEM_POST(sems->roundCheckSema);
	}
	pthread_exit(0);
}

void readerFunction(char* query_block,
                    const std::map<std::string, char *> &dbs,
                    indexT bucketDepth,
                    indexT seqCount,
                    indexT delimiterNum,
                    std::size_t round) {

	readIndex(bucketDepth, seqCount, delimiterNum, dbs, round);

	// Keep a counter which details how long the buffer is and how close are to it.
	// Keep going until we reach that counter, that is the buffer equivalent to a file
	std::size_t counter = 0;
	std::size_t length = strlen(query_block);
	std::size_t total_read = 0;
	char *query_tmp = query_block;

	while (counter <= length) {

		SEM_WAIT(sems->readerSema);
		SEM_WAIT(sems->inputOutputQueueSema);

		int id = idInputQueue.front();
		idInputQueue.pop();
		threadData *data = threadDatas[id];
		MultiSequence *current = inputQueue.front();
		inputQueue.pop();

		SEM_POST(sems->inputOutputQueueSema);

		std::size_t num_sequences_read = 0;
		while (num_sequences_read < INPUT_SIZE && counter <= length){
			size_t oldUnfinishedSize = current->unfinishedSize();

			std::size_t increment = current->appendFromFasta(query_tmp);

			counter += increment;

			// encode the newly-read sequence
			queryAlph->tr(current->seqWriter() + oldUnfinishedSize,
			              current->seqWriter() + current->unfinishedSize());

			num_sequences_read++;
			total_read++;
		}


		data->queryQueue.push(current);
		readSequences++;

		SEM_POST(data->tsems.readSema);
	}

	SEM_WAIT(sems->roundCheckSema);
	roundDone = true;
	SEM_POST(sems->roundCheckSema);

	SEM_WAIT(sems->terminationSema);
}

void Lastal::lastAlign(char *queryblock,
                       char *outputBuffer,
                       std::size_t round,
                       const Representatives &reps)
{
	//!!
	// global to print stuff
	r = round;

	// Flush the queues every round
	// All other queues are dealt with naturally through deletions
	// so we only need to flush the InputQueue
	while (!idInputQueue.empty()) {
		idInputQueue.pop();
	}

	// reset the values
	roundDone = false;
	threadTerminate = false;
	volumes = 1;
	readSequences = 0;
	doneSequences = 0;
	numOfIndexes = 0;
	minScoreGapless = 0;
	sim_cutoff = 0;

	// Set the global similarity cutoff field.
	// Can't refactor it nicely right now, this works.
	sim_cutoff = similarity_cutoff;

	indexT delimiterNum = 0;
	indexT bucketDepth = 0;
	indexT seqCount = 0;
	indexT seqLen = 0;
	createStructures(dbs, bucketDepth, seqCount, delimiterNum,
	                 seqLen, round);

	doneSequences -= (args.threadNum) * 2;

	initializeWorkers(reps);
	pthread_create(&writerThread, 0, __writerFunction, (void *) outputBuffer);

	readerFunction(queryblock, dbs, bucketDepth, seqCount, delimiterNum, round);

	deleteStructures();
}

void deleteStructures() {
	// Terminate the threads
	threadTerminate = true;
	for (int j = 0; j < args.threadNum; j++) {
		SEM_POST(threadDatas[j]->tsems.readSema);
		threadDatas[j]->joinThread();
		delete threadDatas[j];
	}
	delete[] threadDatas;
	pthread_join(writerThread, NULL);

	text->closeFiles();
	suffixArray.closeFiles();
	delete text;

	delete scoreMatrix;
	delete oneQualityScoreMatrix;
	delete twoQualityScoreMatrixMasked;
	delete twoQualityScoreMatrix;
	delete alph;

	delete sems;

	for (int j = 0; j < 2; j++) {
		if(!inputQueue.empty()) {
			delete inputQueue.front();
			inputQueue.pop();
		}
	}
}

// Get the arguments for lastal
Lastal::Lastal(const std::map<std::string, char *> &lastDatabases,
               const Arguments &args) {

	dbs = lastDatabases;
	similarity_cutoff = args.get_similarity();

}
