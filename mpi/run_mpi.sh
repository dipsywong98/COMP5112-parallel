mpic++ -std=c++11 main.cpp mpi_smith_waterman_skeleton.cpp -o mpi_smith_waterman
num=8
# test=./datasets/sample.in
#test=./datasets/1k.in
test=./datasets/20k.in
if [ -n "$1" ]
then
echo $1
num=$1
fi
if [ -n "$2" ]
then
test=$2
fi
echo processes: $num
echo testfile: $test
mpiexec --hostfile ~/hostfile -n $num ./mpi_smith_waterman $test
serial/serial_smith_waterman $test
