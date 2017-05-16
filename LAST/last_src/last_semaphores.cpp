//
// Created by david on 19/02/16.
//

#include "last_semaphores.h"
#include <errno.h>
#include "utilities.hh" // random_str

semaphores_last::semaphores_last(){
	initializeSemaphores();
}

semaphores_last::~semaphores_last(){
	destroySemaphores();
}

void semaphores_last::initializeSemaphores()
{
#ifdef __APPLE__
	sem_unlink("/inputOutputQueueSema");
	if (( inputOutputQueueSema = sem_open("/inputOutputQueueSema", O_CREAT, 0644, 1)) == SEM_FAILED ) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink("/readerSema");
	if (( readerSema = sem_open("/readerSema", O_CREAT, 0644, 0)) == SEM_FAILED ) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink("/writerSema");
	if (( writerSema = sem_open("/writerSema", O_CREAT, 0644, 0)) == SEM_FAILED ) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink("/terminaionSema");
	if (( terminationSema = sem_open("/terminationSema", O_CREAT, 0644, 0)) == SEM_FAILED ) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
	sem_unlink("/roundCheckSema");
	if (( roundCheckSema = sem_open("/roundCheckSema", O_CREAT, 0644, 1)) == SEM_FAILED ) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
#elif __linux
	sem_init(&readerSema, 0, 0);
	sem_init(&writerSema, 0, 0);
	sem_init(&terminationSema, 0, 0);
	sem_init(&roundCheckSema, 0, 1);
	sem_init(&inputOutputQueueSema, 0, 1);
#endif
}

void semaphores_last::destroySemaphores()
{
#ifdef __APPLE__
	sem_unlink("/inputOutputQueueSema");
	sem_unlink("/readerSema");
	sem_unlink("/writerSema");
	sem_unlink("/terminaionSema");
	sem_unlink("/roundCheckSema");
#elif __linux
	sem_destroy(&readerSema);
	sem_destroy(&writerSema);
	sem_destroy(&terminationSema);
	sem_destroy(&roundCheckSema);
	sem_destroy(&inputOutputQueueSema);
#endif
}

semaphores_last_thread::semaphores_last_thread(){
	initializeSemaphores();
}

semaphores_last_thread::~semaphores_last_thread(){
	destroySemaphores();
}

void semaphores_last_thread::generate_named_semaphore(const std::string &prefix,
                                                      SEM_T sem,
                                                      std::size_t c){
#ifdef __APPLE__
	std::size_t count = 20;
	std::string potential_name;
	do {
		count--;
		std::string potential_name = prefix + random_str(count);
		char name[40];
		sprintf(name, "%d", potential_name.c_str());
		sem = sem_open(name, O_CREAT, 0644, c);
		//} while(errno != -1);
	} while (errno != EACCES || errno !=EEXIST || errno !=EINTR || errno !=EINVAL || errno !=EMFILE ||
	 errno !=ENAMETOOLONG || errno !=ENFILE || errno !=ENOENT || errno !=ENOSPC);
	names.push_back(potential_name);
#endif
}

void semaphores_last_thread::initializeSemaphores()
{


#ifdef __APPLE__
	/*
	char name[40];

	sprintf(name, "/readSema%d", identifier);
	sem_unlink(name);
	readSema = sem_open(name, O_CREAT, 0644, 0);

	sprintf(name, "/writeSema%d", identifier);
	sem_unlink(name);
	writeSema = sem_open(name, O_CREAT, 0644, 2);
	*/
	generate_named_semaphore("/readSema", readSema, 0);
	generate_named_semaphore("/writeSema", writeSema, 2);
#elif __linux
	sem_init(&readSema, 0, 0);
	sem_init(&writeSema, 0, 2);
#endif
}

void semaphores_last_thread::destroySemaphores()
{
#ifdef __APPLE__
	/*
	char name[40];

	sprintf(name, "/readSema%d", identifier);
	sem_unlink(name);

	sprintf(name, "/writeSema%d", identifier);
	sem_unlink(name);
	*/
	for(int i=0; i<names.size(); i++){
		sem_unlink(names.back().c_str());
		names.pop_back();
	}
#elif __linux
	sem_destroy(&readSema);
	sem_destroy(&writeSema);
#endif
}
