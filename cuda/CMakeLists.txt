cmake_minimum_required(VERSION 3.16)
set(CMAKE_CUDA_COMPILER /usr/local/cuda-10.2/bin/nvcc)
project(cudacl CUDA)

set(CMAKE_CUDA_STANDARD 11)

add_executable(cudacl main.cu cuda_smith_waterman_skeleton.cu)

set_target_properties(
        cudacl
        PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON)
