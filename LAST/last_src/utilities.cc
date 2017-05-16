#include "utilities.hh"
#include "lastal.hh"

using namespace std;

/*
char *split_n_pick(const string  &strn,  char *buf, char d, unsigned int n) {
	strcpy(buf, strn.c_str());

	char *v=buf;
	char *s1 = buf;
	v=s1;

	unsigned int i =0;

	while(*s1 != '\0') {
		if(*s1==d) {
			*s1 = '\0';
			i++;
			if(i > n) return  v ;
			v=s1+1;
		}
		s1++;
	}
	return v;
}
 */

string random_str(const int len) {
	static const char alphanum[] =
			"0123456789"
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"abcdefghijklmnopqrstuvwxyz";

	string str;

	for (int i = 0; i < len; ++i) {
		str += alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	return str;
}

std::string generate_directory_name(const char *base, const std::string &tmpdir) {
	string random_string;
	int err = -1;
	do{
		random_string = random_str(30);
		string potential_directory = tmpdir + random_string + base;
		struct stat potential_directory_stat;
		err = stat(potential_directory.c_str(), &potential_directory_stat);
	} while(err != -1);
	return random_string;
}

/*
double evalue_extractor_from_blast(const string &line){
	char buf[10000];
	string evaluestr  = split_n_pick(line, buf, '\t', 10);
	double evalue ;

	try{
		evalue = atof(evaluestr.c_str());
	}
	catch(...) {
		return 100;
	}
	return evalue;
}
*/

/*
 * At this point we have sorted for each query all of it's representatives by length
 * Therefore for each query we just need to pick the top hit
 */
void selectBest(std::vector<Match*> *outputVector, const Representatives &reps){
	int count = 0;
	std::string prevMem = "";
	std::string currMem = "" ;

	for (unsigned long i = 0; i < outputVector->size(); i++) {
		Match *current = outputVector->at(i);
		currMem = current->member;

		if(reps.representatives[current->rrank - reps.indexStart]) {
			if (!(currMem.compare(prevMem) == 0 || prevMem.size() == 0)) {
				count = 0;
			}

			if (count > 0) {
				outputVector->at(i)->dead = true;
			}
		} else {
			// hit a representative that's previously been clustered
			outputVector->at(i)->dead = true;
		}

		count++;
		prevMem = currMem;
	}
}

void parseReps(std::vector<Match*> *outputVector, const Representatives &reps) {
	std::string prevRep = "";
	std::string currRep = "";
	unsigned long prevRank = 0;

	std::size_t count = 0;

	for (unsigned long i = 0; i < outputVector->size(); i++) {
		Match *current = outputVector->at(i);
		currRep = current->representative;

		//std::cout << reps.indexStart << std::endl;
		// If the representative changes, update the rank
		if (!(currRep.compare(prevRep) == 0 && prevRep.size() != 0)) {
			reps.representatives[prevRank - reps.indexStart] = true;
			//std::cout << "SET TO TRUE" << std::endl;
		}

		if ((!reps.representatives[current->rrank - reps.indexStart]) && // The rep is not a member
		    (!reps.representatives[current->qrank - reps.indexStart])) { // the member is not clustered
			// hit a query that's eligible
			//std::cout << "SET TO TRUE 2" << std::endl;
			count++;
			reps.representatives[current->qrank - reps.indexStart] = true;
		} else {
			// hit a query that's ineligible due to it's cluster's ineligibility
			outputVector->at(i)->dead = true;
		}

		prevRep = currRep;
		prevRank = current->rrank;
	}

	// take care of the last one
	if(outputVector->size() > 0 && prevRank != 0) {
		reps.representatives[prevRank - reps.indexStart] = true;
	}
}

void faststrcat(char* dst, unsigned long dstLength, char* src, unsigned long srcLength){
	unsigned long srcPos = 0;
	for(unsigned long i=dstLength; i<=dstLength+srcLength; i++){
		dst[i] = src[srcPos];
		srcPos++;
	}
}

void faststrcatString(char* dst, unsigned long dstLength, std::string src){
	unsigned long srcPos = 0;
	unsigned long srcLength = src.size();
	for(unsigned long i=dstLength; i<dstLength+srcLength; i++){
		dst[i] = src[srcPos];
		srcPos++;
	}
	dst[srcLength + dstLength] = NULL;
}
