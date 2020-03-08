/**
 * sgesfegvsedtfsefesegessefe
 * Name: WONG Yuk Chun
 * Student id: 20419764
 * ITSC email: ycwongal@connect.ust.hk
 */

#include <iostream>
#include <sstream>
#include "mpi_smith_waterman.h"

#define max(a, b) (a > b ? a : b)

class Log {
 private:
  bool tempHide = false;

 public:
  bool debug = true;
  std::ostream* pFout = &std::cout;
  void setStream(std::ostream* p) { pFout = p; }
  template <class T>
  Log& operator<<(const T& t) {
    if (debug && !tempHide) {
      (*pFout) << t;
    }
    return *this;
  }
  Log& operator()(int rank, int rank_only = -1) {
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

int smith_waterman(int my_rank, int p, MPI_Comm comm, char* a, char* b,
                   int a_len, int b_len) {
  int max_h = 0, h = 0, temp = 0, prev_h = 0, prev_up = 0;
  MPI_Status status;
  // Log log;
  // log.debug = false;
  MPI_Bcast(&a_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&b_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if(my_rank != 0){
    a = new char[a_len];
    b = new char[b_len];
  }
  MPI_Bcast(a, a_len, MPI_CHAR, 0, MPI_COMM_WORLD);
  // log(my_rank) << "hi\n";
  MPI_Bcast(b, b_len, MPI_CHAR, 0, MPI_COMM_WORLD);

  if (my_rank == 1) {
    MPI_Recv(&temp, 1, MPI_INT, 0, 0, comm, &status);
  } else {
    MPI_Send(&temp, 1, MPI_INT, 1, 0, comm);
  }

  const int parent_rank = (my_rank + p - 1) % p;
  const int child_rank = (my_rank + 1) % p;

  for (int j = my_rank; j < a_len+1; j += p) {
    prev_h = 0;
    prev_up = 0;
    for (int i = 0; i < b_len+1; i++) {
      h = 0;
      if (i > 0 && j > 0) {
        // log(my_rank, 1) << "pu"<< prev_up<<"\n";
        h = max(h, prev_up + sub_mat(a[j - 1], b[i - 1]));
      }
        // log(my_rank, 1) << "1\n";
      if (j > 0) {
        // log(my_rank) << "from " << parent_rank << " tag " << i<< '\n';
        // MPI_Recv(&temp, 1, MPI_INT, 0, 0, comm, &status);
        MPI_Recv(&temp, 1, MPI_INT, parent_rank, i, comm,
                 MPI_STATUS_IGNORE);
        // log(my_rank, 1) << "2\n";
        prev_up = temp;
        h = max(h, temp - GAP);
      }
      h = max(h, prev_h - GAP);

      // log(my_rank, 1) << "send " << child_rank << " tag " << i << '\n';
      MPI_Send(&h, 1, MPI_INT, child_rank, i, comm);
      // log(my_rank, 1) << "3\n";

      prev_h = h;
      max_h = max(max_h, h);
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

  if(my_rank != 0){
    delete[] a;
    delete[] b;
  }

  return max_h;
}
