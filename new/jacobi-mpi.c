/* Jacobi iteration using MPI

  usage (after creating and booting a hosts file -- see lamboot)

    hcc -o jacobi jacobi.c -lmpi
    mpirun -w -c <numWorkers+1> jacobi  -- <gridSize> <iters>   */

#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define MAXGRID 4098     /* maximum grid size (real points plus edges) */
#define COORDINATOR 0   /* process number of the Coordinator */

static void Coordinator(int,int,int);
static void Worker(int,int,int,int,int);
double grid[MAXGRID][MAXGRID];
double grid2[2][MAXGRID][MAXGRID];


/* main() -- initialize MPI, then become one of the processes */
/* lam initiates <numWorkers+1> instances of this routine */

int main(int argc, char *argv[]) {
  int myid;
  int numWorkers, gridSize;  /* assume gridSize is multiple of numWorkers */
  int stripSize;             /* gridSize/numWorkers             */
  int numIters;              /* number of iterations to execute */

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);  /* what is my id (rank)? */
  MPI_Comm_size(MPI_COMM_WORLD, &numWorkers);  /* how many processes? */
  numWorkers--;   /* one coordinator, the other processes are workers */

  /* get command-line arguments and do a simple error check */
  gridSize = atoi(argv[1]);
  numIters = atoi(argv[2]);
  stripSize = gridSize/numWorkers;
  if (gridSize%numWorkers != 0) {
    printf("grid size must be a multiple of number of workers\n");
    exit(1);
  }

  /* become one of the actual processes, depending on my id */
  if (myid == 0) {
    printf("1 Coordinator and %d Workers\n", numWorkers);
    printf("  gridSize:  %d\n  stripSize:  %d\n  numIters:  %d\n",
       gridSize, stripSize, numIters);
    Coordinator(numWorkers, stripSize, gridSize);
  } else {
    Worker(myid, numWorkers, stripSize, gridSize, numIters);
  }

  MPI_Finalize();  /* clean up MPI */
}


/* gather and print results from Workers */
static void Coordinator(int numWorkers, int stripSize, int gridSize) {
  //double grid[MAXGRID][MAXGRID];
  int i, j, startrow, endrow;
  int workerid;
  MPI_Status status;
  FILE *results;
  double mydiff = 0.0, maxdiff = 0.0;
  time_t start, end;
  float time;

  start=clock();
  for (workerid = 1; workerid <= numWorkers; workerid++) {
    startrow = (workerid-1)*stripSize + 1;
    endrow = startrow + stripSize - 1;
    for (i = startrow; i <= endrow; i++) {
        MPI_Recv(&grid[i][1], gridSize, MPI_DOUBLE, workerid, 0,
            MPI_COMM_WORLD, &status);
    }
    printf("got results from worker %d\n", workerid);
  }
  end=clock();
  time=((float)end - (float)start)/CLOCKS_PER_SEC;
  printf("\nTIME: %f\n", time);

  MPI_Reduce(&mydiff, &maxdiff, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  printf("global maxdiff is %f\n", maxdiff);

  /* output the results to file "results" */
  char filename[100];
  sprintf(filename, "results-np%d-%d", numWorkers, gridSize);
  results = fopen(filename, "w");
  for (i = 1; i <= gridSize; i++) {
    for (j = 1; j <= gridSize; j++) {
      fprintf(results, "%f ", grid[i][j]);
    }
    fprintf(results, "\n");
  }
}


/* Each Worker computes values in one strip, communicating with
  neighboring workers until the specified number of iterations.
  Each then sends its results to the Coordinator for printing.
 */

static void Worker(int myid, int numWorkers, int stripSize,
                   int gridSize, int numIters) {
  //double grid [2][MAXGRID][MAXGRID];
  int i, j, iters;
  int current = 0, next = 1;   /* current and next iteration indices */
  int left = 0, right = 0;    /* neighboring strips above and below */
  MPI_Status status;
  double mydiff = 0.0, maxdiff, temp;

  /* set all points to 0s, then left and right edges of grid to 1s
     first worker sets top row to 1s; last worker sets bottom row to 1s */

  for (i = 0; i <= stripSize+1; i++)
    for (j = 0; j <= gridSize+1; j++) {
      grid2[current][i][j] = 0.0;
      grid2[next][i][j] = 0.0;
    }
  for (i = 0; i <= stripSize+1; i++) {
    grid2[current][i][0] = 1.0;
    grid2[current][i][gridSize+1] = 1.0;
    grid2[next][i][0] = 1.0;
    grid2[next][i][gridSize+1] = 1.0;
  }
  if (myid == 1)
    for (j = 0; j <= gridSize+1; j++) {
      grid2[current][0][j] = 1.0;
      grid2[next][0][j] = 1.0;
    }
  if (myid == numWorkers)
    for (j = 0; j <= gridSize+1; j++) {
      grid2[current][stripSize+1][j] = 1.0;
      grid2[next][stripSize+1][j] = 1.0;
    }

  /* determine neighbors */
  if (myid > 1)
    left = (myid-2)%numWorkers + 1;
  if (myid < numWorkers)
    right = myid%numWorkers + 1;

  printf("Worker %d initialized; left is worker %d and right is worker %d\n", 
      myid, left, right);

  /* do the actual computation */
  for (iters = 1; iters <= numIters; iters++) {
    printf("Worker %d Running iter %d\n", myid, iters);
    /* exchange my boundaries with my neighbors, in a ring */
    if (right != 0)
        printf("SEND_START: %d -> %d\n", myid, right);
        MPI_Send(&grid2[next][stripSize][1], gridSize, MPI_DOUBLE, right, 0,
                    MPI_COMM_WORLD);
        printf("SEND_END: %d -> %d\n", myid, right); 
    if (left != 0)
        printf("SEND_START: %d -> %d\n", myid, left);
        MPI_Send(&grid2[next][1][1], gridSize, MPI_DOUBLE, left, 0,
                    MPI_COMM_WORLD);
        printf("SEND_END: %d -> %d\n", myid, left);
    if (left != 0)
        printf("RECV_START: %d <- %d\n", myid, left);
        MPI_Recv(&grid2[next][0][1], gridSize, MPI_DOUBLE, left, 0,
                    MPI_COMM_WORLD, &status);
       printf("RECV_END: %d <- %d\n", myid, left);
       printf("
    if (right != 0)
        printf("RECV_START: %d <- %d\n", myid, right);
        MPI_Recv(&grid2[next][stripSize+1][1], gridSize, MPI_DOUBLE, right, 0,
                    MPI_COMM_WORLD, &status);
        printf("RECV_END: %d <- %d\n", myid, right);

    /* update my points */
    for (i = 1; i <= stripSize; i++) {
      for (j = 1; j <= gridSize; j++) {
        grid2[next][i][j] = (grid2[current][i-1][j] + grid2[current][i+1][j] +
               grid2[current][i][j-1] + grid2[current][i][j+1]) / 4;
      }
    }

    /* swap roles of grids */
    current = next;  next = 1-next;
  }

  /* send results of my current strip to the coordinator */
  printf("WORKER %d SEND TO COORDINATOR", myid);
  for (i = 1; i <= stripSize; i++) {
      MPI_Send(&grid2[current][i][1], gridSize, MPI_DOUBLE,
            COORDINATOR, 0, MPI_COMM_WORLD);
  }
  printf("WORKER %d FINISH SENDING", myid);

  /* compute maximum difference and reduce it with Coordinator */
  for (i = 1; i <= stripSize; i++)
    for (j = 1; j <= gridSize; j++) {
      temp = fabs(grid2[next][i][j] - grid2[current][i][j]);
      if (temp > mydiff)
        mydiff = temp;
    }
  printf("WORKER %d REDUCE TO COORDINATOR", myid);
  MPI_Reduce(&mydiff, &maxdiff, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  printf("WORKER %d FINISH REDUCE", myid);
  printf("maxdiff of worker %d is %f\n", myid, mydiff);

}
