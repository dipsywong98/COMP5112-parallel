echo compiling $1
echo num processes $2
mpic++ -std=c++11 $1
mpiexec --hostfile ~/hostfile -n $2 a.out
