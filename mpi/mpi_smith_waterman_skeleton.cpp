/**
 * sgesfegvsedtfsefesegessefe
 * Name: WONG Yuk Chun
 * Student id: 20419764
 * ITSC email: ycwongal@connect.ust.hk
 */

#include <iostream>
#include <sstream>
#include <cstring>
#include "mpi_smith_waterman.h"

#define max(a, b) (a > b ? a : b)

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
    if ((rank == rank_only || rank_only == -1) && debug == true) {
      (*pFout) << rank << ": ";
      tempHide = false;
    } else {
      tempHide = true;
    }
    return *this;
  }
};

/*
 *  You can add helper functions and variables as you wish.
 */

int smith_waterman(int my_rank, int p, MPI_Comm comm, char *a, char *b,
                   int a_len, int b_len) {
  int max_h = 0, h = 0, temp = 0, prev_h = 0, prev_up = 0;
  int block_width = 50;
  int block_height = 50;
//  int block_send[block_width];
//  int block_receive[block_width];
  int prev_col[block_height] = {};
  int prev_row[block_width] = {};
  int next_col[block_height] = {};
  int block[block_height][block_width] = {};
  int prev_corner = 0;
  MPI_Status status;
  Log log(my_rank);
  // log.debug = false;
  MPI_Bcast(&a_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&b_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (my_rank != 0) {
    a = new char[a_len];
    b = new char[b_len];
  }
  MPI_Bcast(a, a_len, MPI_CHAR, 0, MPI_COMM_WORLD);
  MPI_Bcast(b, b_len, MPI_CHAR, 0, MPI_COMM_WORLD);

  const int parent_rank = (my_rank + p - 1) % p;
  const int child_rank = (my_rank + 1) % p;

  for (int y = my_rank; y < a_len + 1; y += p * block_height) {
    memset(prev_col, block_height, 0);
    for (int x = 0; x < b_len + 1; x += block_width) {
      if (y != 0) {
        MPI_Recv(prev_row, block_width, MPI_INT, parent_rank, x / block_width, comm, MPI_STATUS_IGNORE);
      } else{
        memset(prev_row, block_width, 0);
      }
      const int height = min(block_height, a_len + 1 - y);
      const int width = min(block_width, b_len + 1 - x);
      for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
          const int i = x + dx;
          const int j = y + dy;
          h = 0;
          if (dx == 0) {
            h = max(h, prev_col[dy] - GAP);
          }else{
            h = max(h, block[dy][dx-1] - GAP);
          }
          if (dy == 0) {
            h = max(h, prev_row[dx] - GAP);
          }else{
            h = max(h, block[dy-1][dx] + sub_mat(a[j - 1], b[i - 1]));
          }
          if (dx == 0 && dy == 0) {
            h = max(h, prev_corner + sub_mat(a[j - 1], b[i - 1]));
          }else {
            if(dx != 0 && dy != 0) {
              h = max(h, block[dy-1][dx-1] + sub_mat(a[j - 1], b[i - 1]));
            }else{
              if(dx == 0) {
                h = max(h, prev_col[dy-1] + sub_mat(a[j - 1], b[i - 1]));
              }
              if(dy == 0) {
                h = max(h, prev_row[dx-1] + sub_mat(a[j - 1], b[i - 1]));
              }
            }
          }
          block[dy][dx] = h;
          if(dx == width-1){
            next_col[dy] = h;
          }
        }
      }
      int *tmp = next_col;
      next_col = prev_col;
      prev_col = tmp;
      prev_corner = block[block_height-1][block_width];
      MPI_Send(block[block_height-1], block_width, MPI_INT, child_rank, x / block_width, comm);
    }
  }

  if (my_rank == 0) {
    for (int i = 1; i < p; i++) {
      MPI_Recv(&temp, 1, MPI_INT, i, a_len * b_len, comm, MPI_STATUS_IGNORE);
      max_h = max(max_h, temp);
    }
  } else {
    MPI_Send(&max_h, 1, MPI_INT, 0, a_len * b_len, comm);
  }

  // MPI_Reduce(&max_h, &max_h, 1, MPI_INT, MPI_MAX, 0, comm);

  if (my_rank != 0) {
    delete[] a;
    delete[] b;
  }

  return max_h;
}
