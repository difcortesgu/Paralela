#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv){
    int rank, size;

    

    MPI_Init(&argc, &argv);
        if (rank == 0){
            MPI_Send()
        }else{
            MPI_Recv()
        }
        MPI_Comm_size( MPI_COMM_WORLD, &size );
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );
        printf("Hello world from process %d of %d\n", rank, size);
    MPI_Finalize();
    return 0;
}