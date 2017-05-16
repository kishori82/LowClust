cmake_minimum_required(VERSION 2.6)
project(lc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -w -m64 -pthread -Igumbel_params -I.")

set(LOWCLUST_SOURCE_FILES
        lowclust_src/lowclust.cc
        lowclust_src/arguments.cc
        lowclust_src/fasta.cc
        lowclust_src/lowclust.hh
        lowclust_src/arguments.hh
        lowclust_src/fasta.hh
        lowclust_src/Cluster.cpp
        lowclust_src/Cluster.h
        lowclust_src/stages_master/sortingStep.cpp
        lowclust_src/stages_master/sortingStep.h
        lowclust_src/stages_master/suffixArraysStep.cpp
        lowclust_src/stages_master/suffixArraysStep.h
        lowclust_src/stages_master/clusteringStep.cpp
        lowclust_src/stages_master/clusteringStep.h
        lowclust_src/ioArguments.h
        lowclust_src/semaphore_macros.h
        lowclust_src/stages_master/stepInterface.cpp
        lowclust_src/stages_master/stepInterface.h
        lowclust_src/Match.cpp
        lowclust_src/Match.h
        lowclust_src/sortFasta.cpp
        lowclust_src/sortFasta.h
        lowclust_src/stages_workers/clusteringStepWorker.cpp
        lowclust_src/stages_workers/clusteringStepWorker.h
        lowclust_src/stages_workers/sortingStepWorker.cpp
        lowclust_src/stages_workers/sortingStepWorker.h
        lowclust_src/stages_workers/suffixArraysStepWorker.cpp
        lowclust_src/stages_workers/suffixArraysStepWorker.h
        lowclust_src/stages_workers/workerInterface.cpp
        lowclust_src/stages_workers/workerInterface.h
        lowclust_src/stages_master/semaphores/semaphores_suffix.cpp
        lowclust_src/stages_master/semaphores/semaphores_suffix.h
        lowclust_src/stages_master/buffers/Buffers_SA.cpp
        lowclust_src/stages_master/buffers/Buffers_SA.h
        lowclust_src/stages_master/buffers/Buffers.cpp
        lowclust_src/stages_master/buffers/Buffers.h
        lowclust_src/stages_master/semaphores/semaphores_clustering.cpp
        lowclust_src/stages_master/semaphores/semaphores_clustering.h
        lowclust_src/stages_master/buffers/Buffers_Clustering.cpp
        lowclust_src/stages_master/buffers/Buffers_Clustering.h
        lowclust_src/stages_master/Representatives.cpp
        lowclust_src/stages_master/Representatives.h
        LAST/last_src/last_semaphores.cpp
        LAST/last_src/last_semaphores.h
        LAST/last_src/globals.h
        LAST/last_src/Dispatcher.cpp
        LAST/last_src/Dispatcher.h
        LAST/last_src/lastThreads.cpp
        LAST/last_src/lastThreads.h)


file(GLOB LAST_SOURCE_FILES
        "LAST/last_src/*.hh"
        "LAST/last_src/*.cc"
        )

include_directories(${CMAKE_SOURCE_DIR}/LAST/last_src)
include_directories(${CMAKE_SOURCE_DIR}/lowclust_src)

add_executable(LowClust ${LOWCLUST_SOURCE_FILES} ${LAST_SOURCE_FILES})

find_package(MPI REQUIRED)
include_directories(MPI_INCLUDE_PATH)
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
target_link_libraries(LowClust ${MPI_LIBRARIES})
