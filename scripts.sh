#scripts to run for OpenMP
gcc -o prime_number_parallel_openmp -fopenmp prime_number_parallel_openmp.c -lm

./prime_number_parallel_openmp

#scripts to run for MPI
mpicc prime_number_parallel_mpi.c -o prime_number_parallel_mpi -lm

mpirun -np 4 ./prime_number_parallel_mpi