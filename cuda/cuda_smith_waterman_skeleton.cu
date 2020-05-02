#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>

using namespace std;

#include "cuda_smith_waterman.h"

/*
 *  You can add helper functions and variables as you wish.
 */


#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

inline __device__ int idx(int i, int j, int I) {
  return j * I + i;
}

inline __device__ int get(int* scores, int i, int j, int I) {
  int value;
  while((value = scores[idx(i,j,I)]) == -1);
  return value;
}

__global__ void cuda_sw(char *a, char *b, int a_len, int b_len, int *score, int len, int* max_scores) {
  int tid = blockDim.x * blockIdx.x + threadIdx.x;
  int tn = blockDim.x * gridDim.x;
  int prevy = 0;
  int I = b_len + 1;
  int J = a_len + 1;
  int max_score = 0;
  for (int id = tid; id < len; id += tn) {
    int x = id % I;
    int y = id / I;
//    if (y > prevy) {
//      __syncthreads();
//      prevy++;
//    }
//    if(prevy >= a_len+b_len+1){
//      break;
//    }
//    if(id >= len){
//      continue;
//    }
    int j = y - x;
    int i = x;
    if (i >= 1 && j >= 1 && i < I && j < J) {
      int s = 0;
      if(x>=1 && y >=2){
        int ad = sub_mat(a[i - 1], b[j - 1]);
        s = max(0, get(score,x - 1, y - 2, b_len + 1) + ad); //
      }
      s = max(s, get(score,x - 1, y - 1, b_len + 1) - GAP);
      s = max(s, get(score,x, y - 1, b_len + 1) - GAP);
      max_score = max(max_score, s);
      score[id] = s;
    } else {
      score[id] = 0;
    }

  }
  max_scores[tid] = max_score;
}

__global__ void Kernel() {}

int smith_waterman(int blocks_per_grid, int threads_per_block, char *_a, char *_b, int _a_len, int _b_len) {
  dim3 blocks(blocks_per_grid);
  dim3 threads(threads_per_block);

  char *a;
  char *b;
  int a_len;
  int b_len;
  int len;
  int *d_scores;
  int max_score = 0;
  int tn = blocks_per_grid * threads_per_block;
  int *d_max_scores;
  char *d_a, *d_b;

  a = _a, b = _b, a_len = _a_len, b_len = _b_len;
  if (b_len < a_len) {
    a_len = _b_len;
    b_len = _a_len;
    a = _b;
    b = _a;
  }
  len = ((b_len + 1) * (a_len + 1 + b_len));
//  scores = new int[(sizeof(int) * len)];
  cudaMalloc(&d_scores, sizeof(int) * len);
  cudaMalloc(&d_max_scores, sizeof(int) * tn);
  cudaMalloc(&d_a, sizeof(int) * a_len);
  cudaMemcpy(d_a, a, sizeof(int) * a_len, cudaMemcpyHostToDevice);
  cudaMalloc(&d_b, sizeof(int) * b_len);
  cudaMemcpy(d_b, b, sizeof(int) * b_len, cudaMemcpyHostToDevice);
  cudaMemset(d_scores, 0, sizeof(int) * len);
  cudaMemset(d_max_scores, -1, sizeof(int) * tn);

//  cuda_sw<<<blocks, threads>>>(a, b, a_len, b_len, d_scores, len);
  cuda_sw << < blocks, threads >> > (d_a, d_b, a_len, b_len, d_scores, len, d_max_scores);

  int *h_max = new int[tn];
  int *scores = new int[len];
  cudaMemcpy(h_max, d_max_scores, sizeof(int)*tn, cudaMemcpyDeviceToHost);
  cudaMemcpy(scores, d_scores, sizeof(int)*len, cudaMemcpyDeviceToHost);

  for(int id = 0; id < len; id++){
    if(id % (b_len+1) == 0) std::cout<<std::endl;
    std::cout<<scores[id]<<" ";
  }
  std::cout<<endl;

  for(int i = 0; i< tn; i++){
    max_score = max(max_score, h_max[i]);
  }

//  cudaMemcpy(&max_score, &d_scores[len - 1], sizeof(int), cudaMemcpyDeviceToHost);
  std::cout << cudaGetErrorName(cudaGetLastError()) << std::endl;
  return max_score;
}
