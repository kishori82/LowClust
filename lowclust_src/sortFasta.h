//
// Created by david on 08/01/16.
//

#ifndef LC_SORTFASTA_H
#define LC_SORTFASTA_H

// The base of this code was taken from https://github.com/tdoug870/fasta-parser
#include <string>

extern unsigned long long counter;

	class SortFasta {

			int d_length;
	public:
			char* name_start;
			char* name_end;

			char* seq_start;
			char* seq_end;

			//friend std::istream& operator>>(std::istream&, SortFasta&);

			// Setters
			void set_length(const int &value);
			// Getters
			const int length() const;

			//std::ostream& print(std::ostream& stream) const;
			//std::ostream& count_print(std::ostream& stream) const;
	};

bool compSortFastaPointerLT(const SortFasta *a, const SortFasta *b);

#endif //LC_SORTFASTA_H
