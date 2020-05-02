if [ -n "$4"]
then
  nvcc -std=c++11 main.cu cuda_smith_waterman_skeleton.cu -o cuda_smith_waterman
  else
nvcc -std=c++11 -arch=compute_52 -code=sm_52 main.cu cuda_smith_waterman_skeleton.cu -o cuda_smith_waterman
fi
input=./datasets/1k.in
num_block=4
num_thread=32
if [ -n "$1" ]
then
input=$1
fi

if [ -n "$2" ]
then
num_block=$2
fi

if [ -n "$3" ]
then
num_thread=$3
fi
echo testfile: $input
echo blocks: $num_block
echo thread: $num_thread
./cuda_smith_waterman $input $num_block $num_thread
