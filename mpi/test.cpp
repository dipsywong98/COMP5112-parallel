#include <stdio.h>
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

   int temp = 1;

   if (my_rank != 0) { 
      MPI_Send(&temp, 1, MPI_INT, 0, 0,
            MPI_COMM_WORLD); 
      MPI_Send(&temp, 1, MPI_INT, 0, 0,
            MPI_COMM_WORLD); 
   } else {  
      /* Print my message */
      printf("Greetings from process %d of %d!\n", my_rank, p);
      for (int q = 1; q < p; q++) {
         /* Receive message from process q */
         MPI_Recv(&temp, 1, MPI_INT, q,
            0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         /* Print message from process q */
         printf("%s\n", greeting);
      } 
   }

   /* Shut down MPI */
   MPI_Finalize(); 

   return 0;
}  /* main */
