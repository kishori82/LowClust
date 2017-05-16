#include "Cluster.h"//
#include "Match.h"
// Created by david on 11/11/15.
//

#include <sstream>
#include <iostream>
#include <fstream>

Cluster::Cluster(const std::string &rep) :
		representative(rep)
{ }

void Cluster::addMember(const ClusterMember &member){
	members.push_back(member);
}

//!! Inefficient, spaghetti
void ClusterSet::writeCluster(std::ofstream &o){
	std::map<std::string, Cluster*>::const_iterator it = cSet.begin();
	std::map<std::string, Cluster*>::const_iterator it2 = cSet.end();

	for(; it != it2; ++it){
		o << it->second->representative << "\n";
		std::vector<ClusterMember>::const_iterator vit  = it->second->members.begin();
		std::vector<ClusterMember>::const_iterator vit2 = it->second->members.end();

		for( ; vit!=vit2; ++vit){
			vit->print(o);
		}
	}
}

// Take the buffer from the writerFunction and add it to the clusterSet
//!! Really need to refactor this stringstream out again....
//!! rank, similarity need to be parsed
//!! This entire thing should just be a giant for loop.
void ClusterSet::parseBuffer (char* buffer, bool *statuses, std::size_t indexStart){

	std::stringstream s(buffer);
	std::string line;
	while(getline(s, line)){
		// parse the line
		std::stringstream l(line);

		std::string qname;
		getline(l, qname, '\t');
		//std::cout << qname << std::endl;

		std::string qlength;
		getline(l, qlength, '\t');
		//std::cout << qlength << std::endl;

		std::string qrank;
		getline(l, qrank, '\t');
		std::istringstream i(qrank);
		unsigned long qrank_i;
		i >> qrank_i;
		statuses[qrank_i] = true;
		//std::cout << qrank << std::endl;

		std::string rname;
		getline(l, rname, '\t');
		//std::cout << rname << std::endl;

		std::string rlength;
		getline(l, rlength, '\t');
		//std::cout << rlength << std::endl;

		std::string rrank;
		getline(l, rrank, '\t');
		//std::cout << rrank << "\n\n" << std::endl;
		std::istringstream ii(rrank);
		unsigned long rrank_i;
		ii >> rrank_i;
		statuses[rrank_i] = true;

		std::string sim;
		getline(l, sim);
		//std::cout << rrank << "\n\n" << std::endl;
		std::istringstream iii(sim);
		unsigned long sim_i;
		iii >> sim_i;
		//std::cout << sim_i << "\n\n" << std::endl;

		ClusterMember cm(qname, sim_i);

		addMember(cm, rname, rrank_i, indexStart);
	}
}

void ClusterSet::addMember(const ClusterMember &m,
                           const std::string &representative,
                           unsigned long rrank,
                           std::size_t indexStart){
	try {
		Cluster *ptr = cSet[representative];

		if (ptr == NULL) {
			//std::cout << "NEW, CREATING : " << representative << std::endl;
			//std::cout << indexStart << std::endl;
			potentialReps[rrank-indexStart] = true;
			cSet[representative] = new Cluster(representative);
			cSet[representative]->addMember(m);
		} else {
			//std::cout << "EXISTS, INSERTING into : " << representative << std::endl;
			// insert into the cluster
			cSet[representative]->addMember(m);
		}
	} catch (const std::exception &ex) {
		std::cerr << ex.what() << std::endl;
		std::cerr << "clusterset add member has failed" << std::endl;
	}
}

// http://stackoverflow.com/questions/25968902/destructor-called-when-objects-are-passed-by-value
// Destructors will be called on pass by value arguments which can destroy things
// So I cant use it if it's passed around a lot
//ClusterSet::~ClusterSet(){ }

void ClusterSet::destroy() {
	std::map< std::string, Cluster* >::iterator it = cSet.begin();
	std::map< std::string, Cluster* >::iterator it2 = cSet.end();
	for(; it != it2; ++it){
		if (it->second != NULL) {
			delete it->second;
		}
	}
	delete[] potentialReps;
}


ClusterSet::ClusterSet(std::size_t nr) : numReps(nr) {
	potentialReps = new bool[nr]();
}

ClusterMember::ClusterMember(const std::string &m, float s) :
		member(m), similarity(s) {}

std::ofstream& ClusterMember::print(std::ofstream &o) const{
	o << "\t" << member << "\t" << similarity << "\n";
	return o;
}
