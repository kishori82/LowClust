//
// Created by david on 07/01/16.
//

#include <sstream>
#include <cstdio> //sprintf
#include <iostream>
#include <cstring> //memset
#include "Match.h"
#include "utilities.hh" //faststrcat

bool Match::operator<(const Match &m) const {

/*
	std::cout << member << std::endl;
	std::cout << m.member << std::endl;
	std::cout << rep_length << std::endl;
	std::cout << m.rep_length << std::endl;
	std::cout << similarity << std::endl;
	std::cout << m.similarity << std::endl;
 */

	// Compare by Member, Representative and Similarity
	if(member < m.member) return true;
	if(member > m.member) return false;
	if(rep_length < m.rep_length) return true;
	if(rep_length > m.rep_length) return false;
	if(similarity < m.similarity) return true;
	if(similarity > m.similarity) return false;


	/*
	std::cout << member;
	std::cout << m.member;
	std::cout << rep_length << std::endl;
	std::cout << m.rep_length << std::endl;
	std::cout << similarity << std::endl;
	std::cout << m.similarity << "\n\n" << std::endl;
	 */

	// the default should be false according to strict weak ordering. This is important!
	// http://www.sgi.com/tech/stl/StrictWeakOrdering.html
	return false;
}

bool Match::operator>(const Match &m) const {

	// Compare by Member, Representative and Similarity
	if(member > m.member) return true;
	if(member < m.member) return false;
	if(rep_length > m.rep_length) return true;
	if(rep_length < m.rep_length) return false;
	if(similarity > m.similarity) return true;
	if(similarity < m.similarity) return false;


	//std::cout << "EVER REACH HERE" << std::endl;
	return false;
}

//!! This function is so inefficient, mom's spaghetti code for now
void Match::print(){

	// mem, rep, sim
	std::stringstream s;
	s << member.substr(0, member.size()-1) << "\t";
	s << representative.substr(0, representative.size()-1) << "\t";
	s << similarity << "\n";

	std::cout << s.str();
}

unsigned long Match::strcat(char *dst, unsigned long dstLength, const Match *wr) {

	//std::cout << "STARTING HERE" << std::endl;
	unsigned long memLength = 0;
	for (unsigned long i = dstLength; i < dstLength + wr->member.size(); i++) {
		dst[i] = wr->member[memLength];
		memLength++;
	}
	dstLength += wr->member.size();
	dst[dstLength] = '\t';
	dstLength++;

	unsigned long repLength = 0;
	// -1 because the representative has a \n appended to it
	for (unsigned long i = dstLength; i < dstLength + wr->representative.size() - 1; i++) {
		dst[i] = wr->representative[repLength];
		repLength++;
	}
	dstLength += wr->representative.size() - 1;
	//dst[dstLength] = '\n';
	//dstLength++;
	dst[dstLength] = NULL;

	//printf(dst, "%s");
	//std::cout << "DSTLENGTH : " << dstLength << std::endl;
	//std::cout << "ENDING HERE" << std::endl;

/*
	char buffer [4];
	memset(buffer, 0, 4);
	sprintf (buffer, "%d", wr->similarity);

	faststrcat(dst, dstLength, buffer, 4);
	dstLength += 4;
	*/
	return dstLength;
}

std::string Match::giveFields(){

	// mem, rep, sim
	std::stringstream s;
	s << member.substr(0, member.size()-1) << "\t";
	s << representative.substr(0, representative.size()-1) << "\t";
	s << similarity << "\n";

	return s.str();
}

/*
std::string Match::get_representative() const{
	return representative;
}
std::string Match::get_member() const{
	return member;
}
unsigned Match::get_rep_length() const{
	return rep_length;
}
unsigned long Match::get_qrank() const{
	return qrank;
}
unsigned long Match::get_rrank() const{
	return rrank;
}
float Match::get_similarity() const{
	return similarity;
}
*/

//!! The calculate functions and the alignment function all be significantly cleaned up to be
// basically one function with some helpers.
//!! Too many calls creating temporary objects, deal with creating strings etc.

//!! This function is extremely inefficient but it works for now. It seriously needs to be rewritten.
float Match::calculateIdentity(const std::string &q_name, unsigned total_length){

	//std::cout << q_name << std::endl;

	// Strip the length of the query from it's name
	int	original_length = 0;
	unsigned long index = q_name.find("\t");
	unsigned long end_index = q_name.rfind("\t");
	std::string length = "";
	for(unsigned long i=index+1; i<end_index; i++){
		length += q_name[i];
	}
	std::istringstream(length) >> original_length;

	// Calculate percent identity
	std::stringstream i;
	i << ((double) 100 * (double)(total_length) / (double)original_length) << "\n";
	float identity = 0;
	i >> identity;

	//std::cout << q_name << std::endl;
	//std::cout << identity << std::endl;

	return identity;
}

//!! This function is extremely inefficient but it works for now. It seriously needs to be rewritten.
unsigned long Match::calculateRank(const std::string &q_name){

	// Strip the length of the query from it's name
	unsigned long index = q_name.rfind("\t");
	std::string s_rank = "";
	for(unsigned long i = index+1; i<q_name.size(); i++){
		s_rank += q_name[i];
	}

	// Recover the rank
	unsigned long rank;
	std::istringstream(s_rank) >> rank;

	//std::cout << q_name << std::endl;
	//std::cout << rank << std::endl;

	return rank;
}

std::istream& operator >> (std::istream& stream, Match& rhs)
{
	std::string line;
	// obtain one line which is one match
	if (getline(stream, line)) {
		std::stringstream s(line);
		s >> rhs.member;
		s >> rhs.qrank; // query length
		s >> rhs.qrank;
		s >> rhs.representative;
		s >> rhs.rep_length;
		s >> rhs.rrank;
		s >> rhs.similarity;
	}
	return stream;
}

Match::Match(){}

Match::Match(const std::string &r,
             const std::string &m,
             unsigned rl,
             unsigned long q_rank,
             unsigned long r_rank,
             float s):
		representative(r),
		member(m),
		rep_length(rl),
		qrank(q_rank),
		rrank(r_rank),
		similarity(s),
		dead(false){ }
