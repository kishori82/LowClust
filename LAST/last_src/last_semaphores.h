//
// Created by david on 19/02/16.
//

#ifndef LC_LAST_SEMAPHORES_H
#define LC_LAST_SEMAPHORES_H

#include <semaphores.hh>
#include <vector>
#include <string>

class semaphores_last {
public:

		SEM_T readerSema;
		SEM_T writerSema;
		SEM_T terminationSema;
		SEM_T roundCheckSema;
		SEM_T inputOutputQueueSema;

		semaphores_last();
		~semaphores_last();

private:
		void initializeSemaphores();
		void destroySemaphores();
};

class semaphores_last_thread {
public:

		SEM_T readSema;
		SEM_T writeSema;
		SEM_T readerSema;

		semaphores_last_thread();
		~semaphores_last_thread();

private:
		std::vector<std::string> names;

		void initializeSemaphores();
		void destroySemaphores();


		void generate_named_semaphore(const std::string &prefix, SEM_T sem, std::size_t c);
};

#endif //LC_LAST_SEMAPHORES_H
