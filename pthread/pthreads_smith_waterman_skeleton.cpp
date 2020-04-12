/**
 * Name: WONG Yuk Chun
 * Student id: 20419764
 * ITSC email: ycwongal@connect.ust.hk
*/

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <cstring>
#include "pthreads_smith_waterman.h"

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

class Log {
private:
  bool tempHide = false;

public:
  bool debug = true;
  int rank;

  Log(int rank) : rank(rank) {}

  std::ostream *pFout = &std::cout;

  void setStream(std::ostream *p) { pFout = p; }

  template<class T>
  Log &operator<<(const T &t) {
    if (debug && !tempHide) {
      (*pFout) << t;
    }
    return *this;
  }

  Log &operator()(int rank_only = -1) {
    if ((rank == rank_only || rank_only == -1) && debug) {
      (*pFout) << rank << ": ";
      tempHide = false;
    } else {
      tempHide = true;
    }
    return *this;
  }
};

// global
char *a;
char *b;
int a_len;
int b_len;
int num_threads;
sem_t *sem;
//int** d;
int *max_H;
int *Left;

void *Pth_smith_waterman(void *pRank) {
  int my_rank = (long) (pRank);
  int block_size = max(50, (b_len + 1) / num_threads);
  int x_start = my_rank * block_size;
  int x_end = min((my_rank + 1) * block_size, b_len + 1);
  int x_cnt = x_end - x_start;
  int h = 0;
  long max_h = 0;
  int *currRow = new int[block_size];
  int *prevRow = new int[block_size];
  memset(prevRow, block_size * sizeof(int), 0);
  int currLeft = 0;
  int prevLeft = 0;
  for (int j = 0; j < a_len + 1; j++) {
    memset(currRow, block_size * sizeof(int), 0);
    if (my_rank != 0 || j != 0) {
      sem_wait(&sem[my_rank]);
      prevLeft = currLeft;
      currLeft = Left[my_rank];
    }
    for (int x = 0; x < x_cnt; x++) {
      int i = x + x_start;
      h = 0;
      if( i>0 && j>0) {
        h = max(h, prevRow[x] - GAP);
        if (x > 0) {
          h = max(h, currRow[x - 1] - GAP);
          h = max(h, prevRow[x - 1] + sub_mat(a[j - 1], b[i - 1]));
        } else {
          h = max(h, currLeft - GAP);
          h = max(h, prevLeft + sub_mat(a[j - 1], b[i - 1]));
        }
      }
      currRow[x] = h;
      max_h = max(max_h, h);
    }

//    Log log(my_rank);
//    log() << "hello" << j << "\n";
//    for(int i = 0; i<10000; i++);
    if (my_rank != num_threads - 1) {
      Left[my_rank + 1] = currRow[block_size - 1];
      sem_post(&sem[my_rank + 1]);
    }else{
      sem_post(&sem[0]);
    }
    int *tmp = currRow;
    currRow = prevRow;
    prevRow = tmp;
  }
  max_H[my_rank] = max_h;
}

/*
 *  You can add helper functions and variables as you wish.
 */


int smith_waterman(int _num_threads, char *_a, char *_b, int _a_len, int _b_len) {
  /*
   *  Please fill in your codes here.
   */
  a = _a, b = _b, a_len = _a_len, b_len = _b_len, num_threads = _num_threads;
  if (b_len < a_len) {
    a_len = _b_len;
    b_len = _a_len;
    a = _b;
    b = _a;
  }

  max_H = new int[num_threads];
  Left = new int[num_threads];

  memset(Left, num_threads * sizeof(int), 0);

  pthread_t thread_handles[num_threads];
  sem = new sem_t[num_threads];
  for (long rank = 0; rank < num_threads; rank++) {
    sem_init(&sem[rank], 0, 0);
    pthread_create(&thread_handles[rank], NULL, Pth_smith_waterman, (void *) rank);
  }
  long ret = 0;
  for (long rank = 0; rank < num_threads; rank++) {
    pthread_join(thread_handles[rank], NULL);
    ret = max(ret, max_H[rank]);
  }
  delete[] sem;
  delete[] max_H;
  delete[] Left;
  return ret;
}
