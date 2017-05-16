//
// Created by david on 11/11/15.
//

#ifndef LC_CLUSTER_H
#define LC_CLUSTER_H

#include <string>
#include <vector>
#include <map>

class ClusterMember {
public:
		std::string member;
		float similarity;

		ClusterMember(const std::string &m, float s);
		std::ofstream& print(std::ofstream &o) const;
};

class Cluster {
public:
		std::string representative;
		std::vector<ClusterMember> members;
		Cluster(const std::string &rep);

		void addMember(const ClusterMember &member);
};

class ClusterSet {
private:
		std::map< std::string, Cluster* > cSet;
public:

		bool* potentialReps;
		std::size_t numReps;

		void parseBuffer (char* buffer, bool *statuses,
		                  std::size_t indexStart);

		void addMember(const ClusterMember &m,
		               const std::string &representative,
		               unsigned long rrank,
		               std::size_t indexStart);


		void writeCluster(std::ofstream &o);

		void destroy();

		ClusterSet(std::size_t nr);
};
#endif //LC_CLUSTER_H
