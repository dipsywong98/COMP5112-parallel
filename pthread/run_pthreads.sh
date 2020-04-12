g++ -std=c++11 -pthread main.cpp pthreads_smith_waterman_skeleton.cpp -o pthreads_smith_waterman
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
./pthreads_smith_waterman $test $num
serial/serial_smith_waterman $test
