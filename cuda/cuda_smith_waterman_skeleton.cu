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

__global__ void cuda_sw(char *a, char *b, int a_len, int b_len, int *score, int *max_scores, int y) {
  int tid = blockDim.x * blockIdx.x + threadIdx.x;
  int tn = blockDim.x * gridDim.x;
  int max_score = 0;
  for (int x = tid; x < b_len + 1; x += tn) {
    int j = y - x + 1;
    int i = x;
    int s = 0;
    if (i >= 1 && j >= 1 && i <= b_len && j <= a_len) {
      if (y >= 2) {
        int ad = sub_mat(a[j - 1], b[i - 1]);
        s = max(0, score[idx(x - 1, y - 2, b_len + 1)] + ad);
      }
      s = max(s, score[idx(x - 1, y - 1, b_len + 1)] - GAP);
      s = max(s, score[idx(x, y - 1, b_len + 1)] - GAP);
      max_score = max(max_score, s);
    }
    score[idx(x, y, b_len + 1)] = s;
  }
  max_scores[tid] = max(max_scores[tid], max_score);
}

__global__ void maxOf(int *max_scores) {
  int tid = blockDim.x * blockIdx.x + threadIdx.x;
  int tn = blockDim.x * gridDim.x;
  int l = tn;
  while (l > 1) {
    __syncthreads();
    l = (l + 1) / 2;
    if (tid < l && tid + l < tn) {
      max_scores[tid] = max(max_scores[tid], max_scores[tid + l]);
    }
  }
}

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
  cudaMemset(d_max_scores, 0, sizeof(int) * tn);

//  cuda_sw<<<blocks, threads>>>(a, b, a_len, b_len, d_scores, len);
  for (int y = 0; y < a_len + b_len + 1; y++) {
    cuda_sw <<< blocks, threads >>>(d_a, d_b, a_len, b_len, d_scores, d_max_scores, y);
  }
  maxOf<<< blocks, threads >>>(d_max_scores);

  int *h_max_scores = new int[tn];
//  int *scores = new int[len];
  cudaMemcpy(h_max_scores, d_max_scores, sizeof(int) * tn, cudaMemcpyDeviceToHost);

//  cudaMemcpy(scores, d_scores, sizeof(int) * len, cudaMemcpyDeviceToHost);
//  for (int id = 0; id < len; id++) {
//    if (id % (b_len + 1) == 0) std::cout << std::endl;
//    std::cout << scores[id] << " ";
//  }
//  std::cout << endl;

  for (int i = 0; i < tn; i++) {
//    std::cout << h_max_scores[i] << " ";
    max_score = max(max_score, h_max_scores[i]);
  }

//  cudaMemcpy(&max_score, &d_scores[len - 1], sizeof(int), cudaMemcpyDeviceToHost);
//  std::cout << cudaGetErrorName(cudaGetLastError()) << std::endl;
  return max_score;
}
