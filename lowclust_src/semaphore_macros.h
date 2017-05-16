//
// Created by david on 28/12/15.
//

#ifndef LC_SEMAPHORE_MACROS_H
#define LC_SEMAPHORE_MACROS_H

#include <semaphore.h>

#ifdef __APPLE__
typedef sem_t* SEM_T;
#define SEM_POST(x) sem_post(x)
#define SEM_WAIT(x) sem_wait(x)
#elif __linux
typedef sem_t SEM_T;
#define SEM_POST(x) sem_post(&x)
#define SEM_WAIT(x) sem_wait(&x)
#define SEM_INIT(x, y, z) sem_init(&x, y, z)
#endif

#endif //LC_SEMAPHORE_MACROS_H
