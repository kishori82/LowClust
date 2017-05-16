//
// Created by david on 08/01/16.
//

#include "sortFasta.h"

void SortFasta::set_length(const int& value){
		d_length = value;
	}

	// ACCESSORS

	const int SortFasta::length() const{
		return d_length;
	}

bool compSortFastaPointerLT(const SortFasta *a, const SortFasta *b)
{
	return a->length() < b->length();
}
