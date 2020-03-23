#include <iostream>
#include <string.h>  /* For strlen             */
#include <mpi.h>     /* For MPI functions, etc */ 

const int MAX_STRING = 100;

int main(void) {
   char       greeting[MAX_STRING];  /* String storing message */
   int        p;               /* Number of processes    */
   int        my_rank;               /* My process rank        */

   /* Start up MPI */
   MPI_Init(NULL, NULL); 

   /* Get the number of processes */
   MPI_Comm_size(MPI_COMM_WORLD, &p); 

   /* Get my rank among all the processes */
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

   int sum = 1;

   if(my_rank == 0) {
      int a = 1;
      MPI_Reduce(&a, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
   }else if(my_rank == 1) {
      int b = 1;
      MPI_Reduce(&b, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
   } else if(my_rank == 2) {
      
   }

   /* Shut down MPI */
   MPI_Finalize(); 

   return 0;
}  /* main */
