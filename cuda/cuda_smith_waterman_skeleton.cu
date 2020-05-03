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

__global__ void cuda_sw(char *a, char *b, int a_len, int b_len, int *d_scores, int *max_scores, int y, int* d_scores1, int* d_scores2) {
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
        s = max(0, d_scores2[x-1] + ad);
      }
      s = max(s, d_scores1[x-1] - GAP);
      s = max(s, d_scores1[x] - GAP);
      max_score = max(max_score, s);
    }
    d_scores[x] = s;
  }
  max_scores[tid] = max(max_scores[tid], max_score);
}

int smith_waterman(int blocks_per_grid, int threads_per_block, char *_a, char *_b, int _a_len, int _b_len) {
  dim3 blocks(blocks_per_grid);
  dim3 threads(threads_per_block);

  char *a;
  char *b;
  int a_len;
  int b_len;
  int *d_scores, *d_scores1, *d_scores2;
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
  cudaMalloc(&d_scores, sizeof(int) * (b_len + 1));
  cudaMalloc(&d_scores1, sizeof(int) * (b_len + 1));
  cudaMalloc(&d_scores2, sizeof(int) * (b_len + 1));
  cudaMalloc(&d_max_scores, sizeof(int) * tn);
  cudaMalloc(&d_a, sizeof(int) * a_len);
  cudaMalloc(&d_b, sizeof(int) * b_len);
  cudaMemcpy(d_a, a, sizeof(int) * a_len, cudaMemcpyHostToDevice);
  cudaMemcpy(d_b, b, sizeof(int) * b_len, cudaMemcpyHostToDevice);
  cudaMemset(d_scores, 0, sizeof(int) * (b_len+1));
  cudaMemset(d_scores1, 0, sizeof(int) * (b_len+1));
  cudaMemset(d_scores2, 0, sizeof(int) * (b_len+1));
  cudaMemset(d_max_scores, 0, sizeof(int) * tn);

  for (int y = 0; y < a_len + b_len + 1; y++) {
    cuda_sw <<< blocks, threads >>>(d_a, d_b, a_len, b_len, d_scores, d_max_scores, y, d_scores1, d_scores2);
    int* t = d_scores2;
    d_scores2 = d_scores1;
    d_scores1 = d_scores;
    d_scores = t;
  }

  int *h_max_scores = new int[tn];
  cudaMemcpy(h_max_scores, d_max_scores, sizeof(int) * tn, cudaMemcpyDeviceToHost);

  for (int i = 0; i < tn; i++) {
    max_score = max(max_score, h_max_scores[i]);
  }

  return max_score;
}
