//
// Created by david on 07/01/16.
//

#ifndef LC_MATCH_H
#define LC_MATCH_H

#include <string>

class Match {
private:
public:
		std::string representative;
		std::string member;
		unsigned rep_length;
		unsigned long qrank;
		unsigned long rrank;
		float similarity;

		bool dead;

		Match();
		Match(const std::string &r,
		      const std::string &m,
		      unsigned rl,
		      unsigned long q_rank,
		      unsigned long r_rank,
		      float s);

		bool operator < (const Match& m) const;
		bool operator > (const Match& m) const;


		/*
		std::string get_representative() const;
		std::string get_member() const;
		unsigned get_rep_length() const;
		unsigned long get_qrank() const;
		unsigned long get_rrank() const;
		float get_similarity() const;
		*/

		void print();
		std::string giveFields();

		// Some static functions to help process things
		static float calculateIdentity(const std::string &q_name, unsigned total_length);
		static unsigned long calculateRank(const std::string &q_name);
		static unsigned long strcat(char* dst, unsigned long dstLength, const Match *wr);

};

std::ostream& operator << (std::ostream& os, const Match &rhs);
std::istream& operator >> (std::ostream& is, const Match &rhs);

#endif //LC_MATCH_H
