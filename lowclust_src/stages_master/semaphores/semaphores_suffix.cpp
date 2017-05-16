//
// Created by david on 13/02/16.
//

#include <semaphore_macros.h>
#include "semaphores_suffix.h"

semaphores_suffix::semaphores_suffix(){
	initializeSemaphores();
}


semaphores_suffix::~semaphores_suffix(){
	destroySemaphores();
}

void semaphores_suffix::initializeSemaphores(){
#ifdef __APPLE__
	sem_unlink("/reader");
  if ( ( reader = sem_open("/reader", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/writer");
  if ( ( writer = sem_open("/writer", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/IO");
  if ( ( IO = sem_open("/IO", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/finished_reading_sem");
  if ( ( finished_reading_sem = sem_open("/finished_reading_sem", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/finished_IO");
  if ( ( finished_IO = sem_open("/finished_IO", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/input_ready_sem");
  if ( ( input_ready_sem = sem_open("/input_ready_sem", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/output_empty");
  if ( ( output_empty = sem_open("/output_empty", O_CREAT, 0644, 0)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/input_buf");
  if ( ( input_buf = sem_open("/input_buf", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/empty_input_buf");
  if ( ( empty_input_buf = sem_open("/empty_input_buf", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/output_buf");
  if ( ( output_buf = sem_open("/output_buf", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/empty_output_buf");
  if ( ( empty_output_buf = sem_open("/empty_output_buf", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/read_input_sem");
  if ( ( read_input_sem = sem_open("/read_input_sem", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  sem_unlink("/written_output_sem");
  if ( ( written_output_sem = sem_open("/written_outputer_sem", O_CREAT, 0644, 1)) == SEM_FAILED ) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
#elif __linux
	SEM_INIT(reader, 0, 0);
	SEM_INIT(writer, 0, 0);
	SEM_INIT(IO, 0, 1);
	SEM_INIT(finished_reading_sem, 0, 1);
	SEM_INIT(finished_IO, 0, 0);
	SEM_INIT(input_ready_sem, 0, 0);
	SEM_INIT(output_empty, 0, 0);
	SEM_INIT(input_buf, 0, 1);
	SEM_INIT(empty_input_buf, 0, 1);
	SEM_INIT(output_buf, 0, 1);
	SEM_INIT(empty_output_buf, 0, 1);
	SEM_INIT(read_input_sem, 0, 1);
	SEM_INIT(written_output_sem, 0, 1);
#endif
}

void semaphores_suffix::destroySemaphores() {
#ifdef __APPLE__
	sem_unlink("/reader");
  sem_unlink("/writer");
  sem_unlink("/IO");
  sem_unlink("/finished_reading_sem");
  sem_unlink("/finished_IO");
  sem_unlink("/input_ready_sem");
  sem_unlink("/output_empty");
  sem_unlink("/input_buf");
  sem_unlink("/empty_input_buf");
  sem_unlink("/output_buf");
  sem_unlink("/empty_output_buf");
  sem_unlink("/read_input_sem");
  sem_unlink("/written_output_sem");
#elif __linux
	sem_destroy(&reader);
	sem_destroy(&writer);
	sem_destroy(&IO);
	sem_destroy(&finished_reading_sem);
	sem_destroy(&finished_IO);
	sem_destroy(&input_ready_sem);
	sem_destroy(&output_empty);
	sem_destroy(&input_buf);
	sem_destroy(&empty_input_buf);
	sem_destroy(&output_buf);
	sem_destroy(&empty_output_buf);
	sem_destroy(&read_input_sem);
	sem_destroy(&written_output_sem);
#endif
}
