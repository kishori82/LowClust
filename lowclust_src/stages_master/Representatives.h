//
// Created by david on 15/02/16.
//

#ifndef LC_REPRESENTATIVES_H
#define LC_REPRESENTATIVES_H


class Representatives {
//private:
public:
		bool* representatives;
		size_t indexStart;
		size_t numReps;
		bool repClustering;

public:
		void receiveRepresentativeStatuses();

		Representatives(); // For Worker, fields filled in by the receive function
		Representatives(bool* reps, size_t is, size_t nr); // for Master
		void destroy();
		~Representatives();
};


#endif //LC_REPRESENTATIVES_H
