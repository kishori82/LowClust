#!/bin/bash

#mpirun -np 4 ./LowClust -i datasets/amino_acid_500.fna -o output -b 1 -s 30 -v
#mpirun -np 10 ./LowClust -i datasets/big2.fna -o output -b 1 -s 30 -v
mpirun -np 20 ./LowClust -i ~/Tara_Ocean/output/GOV_Assembly/orf_prediction/GOV_Assembly.faa -o output -b 1 -s 30 -v
