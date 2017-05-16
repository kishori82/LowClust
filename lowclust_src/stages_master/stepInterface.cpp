//
// Created by david on 28/12/15.
//

#include "stepInterface.h"

void* stepInterface::readerEntry(void *args){
	((stepInterface*)args)->readerFunction();
	return NULL;
}

void* stepInterface::writerEntry(void *args){
	((stepInterface*)args)->writerFunction();
	return NULL;
}

void* stepInterface::networkEntry(void *args){
	((stepInterface*)args)->networkFunction();
	return NULL;
}

void stepInterface::startThreads(){
	pthread_create(&r_thread, NULL, readerEntry,  this);
	pthread_create(&w_thread, NULL, writerEntry,  this);
	pthread_create(&n_thread, NULL, networkEntry, this);
}

void stepInterface::joinThreads(){
	pthread_join(r_thread, NULL);
	pthread_join(w_thread, NULL);
	pthread_join(n_thread, NULL);
}

void stepInterface::runStep(){
	startThreads();
	joinThreads();
}
