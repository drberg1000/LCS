#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))

#define gk_clearwctimer(tmr) (tmr = 0.0)
#define gk_startwctimer(tmr) (tmr -= gk_WClockSeconds())
#define gk_stopwctimer(tmr)  (tmr += gk_WClockSeconds())
#define gk_getwctimer(tmr)   (tmr)

/******************************************************************
* Structs containing LCS information and arguments for            *
******************************************************************/
typedef struct LCS_t
{
    char *x;
    char *y;
    short int **c;
    int   m;
    int   n;
} LCS_t;

double gk_WClockSeconds(void);
void LCS_length(LCS_t *, int, int);
void LCS_print(short int **c, char *x, int i, int j);
char* LCS_read(char *);
void distribute_data(LCS_t *, char *, int, int);
void print_c(LCS_t *, int, int);


/*********************************************************
* Use MPI to find LCS length and track back to print LCS *
*********************************************************/
int main(int argc, char** argv)
{

    int num_procs, myid;
    double timer_total;
    LCS_t LCS_info;
    char* string1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if( myid == 0 ) {
        /* Get filenames from argv */
        if( argc != 3 )
        {
            printf("Usage: %s file1 file2\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD,0);
        }

        gk_clearwctimer(timer_total);
        gk_startwctimer(timer_total);

        /* Read from File */
        string1    = LCS_read(argv[1]);
        LCS_info.y = LCS_read(argv[2]);

        LCS_info.m = (int) strlen(string1);
        LCS_info.n = (int) strlen(LCS_info.y);
    }

    /* Broadcast y & Scatter x */
    distribute_data(&LCS_info, string1, myid, num_procs );

    /* Create & Initialize c matrices */
    int i;
    LCS_info.c = malloc(sizeof(int*)*LCS_info.m+1);
    for(i = 0; i <= LCS_info.m; i++) {
        LCS_info.c[i] = malloc(sizeof(int)*LCS_info.n+1);
        LCS_info.c[i][0] = 0;  // set first col to 0
    }
    if (myid == 0) {
        for (i = 0; i <= LCS_info.n; i++)
            LCS_info.c[0][i] = 0; // set first row to 0
    }


    LCS_length( &LCS_info, myid, num_procs );

//    print_c( &LCS_info, myid, num_procs );

    /* Gather c data from processes */
    /* FIXME the gather operation needs to be replaced by a new LCS_Print
       that uses a distributed serial format.  Will start with process p
       and progress to process 0. */



    /* Print Execution times & LCS */
    if( myid == num_procs - 1 )
        printf("Length of LCS: %d\n", LCS_info.c[LCS_info.m][LCS_info.n]);
    if( myid == 0 ) {
        //LCS_print(LCS_info.c, LCS_info.x, LCS_info.m, LCS_info.n);
        gk_stopwctimer(timer_total);
        printf("Number of threads: %d\n", num_procs);
        printf("Time Taken Overall      : %f sec\n", timer_total);
        //printf("Length of LCS: %d\n", LCS_info.c[LCS_info.m][LCS_info.n]);
    }

    MPI_Finalize();

    return 0;

} //END MAIN

void print_c(LCS_t *LCS_info, int myid, int num_procs) {
    int p, i, j;

    for(p = 0; p < num_procs; p++) {
        if (p == myid){
            for(i = 0; i<=LCS_info->m; i++)
            {
                printf("p%d:",myid);
                for(j = 0; j<=LCS_info->n; j++)
                    printf("%3d ", LCS_info->c[i][j]);
                printf("\n");
            }
            fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

/****************************************************
*  Read a file into a char string ignoring newlines *
****************************************************/
char* LCS_read(char *infile) {
    FILE *FIN;
    char c = 'n';
    char *S = NULL;
    char *T;
    int count=0;
    int buffsize=0;
    int num_procs;
    int i;

    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if( (FIN = fopen(infile, "r")) == NULL ) {
        printf("Problem opening %s.\n", infile);
        MPI_Abort(MPI_COMM_WORLD,0);
    }

    while ( (c=fgetc(FIN)) != EOF ) {
        if( c != '\n' ) { /* Ignore new lines & EOF */
            count++;

            if( count >= buffsize ){
                buffsize += num_procs;
                T = (char *) realloc((void *) S, buffsize*sizeof(char));
                if( T != NULL) {
                    S = T;
                    memset(&S[buffsize-num_procs], '\0', num_procs);
                }
                else {
                    free(S);
                    printf("Error (re)allocating memory to store string in %s\n", infile);
                    MPI_Abort(MPI_COMM_WORLD,0);
                }
            }

            S[count-1] = c;
        }
    }

    fclose(FIN);

    return S;
} //end LCS_read



/******************************************************************
* Distribute data to necessary processes                          *
******************************************************************/
void distribute_data(LCS_t *LCS_info, char* string1,
                     int myid, int num_procs ) {

    /* Broadcast all of y to each process */
    MPI_Bcast( &LCS_info->n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if( myid != 0 )
        LCS_info->y = malloc(LCS_info->n*sizeof(char));
    MPI_Bcast( LCS_info->y, LCS_info->n, MPI_CHAR, 0, MPI_COMM_WORLD );


    /* Scatter x to processes */
    MPI_Bcast( &LCS_info->m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int recvsize = (num_procs + LCS_info->m - LCS_info->m%num_procs) / num_procs;
    LCS_info->x = malloc((recvsize+1)*sizeof(char));
    bzero(LCS_info->x, recvsize+1);
    MPI_Scatter( string1   , recvsize, MPI_CHAR,
                 LCS_info->x, recvsize, MPI_CHAR, 0, MPI_COMM_WORLD);
    LCS_info->m = (int) strlen(LCS_info->x);

    return;
}



/******************************************************************
* George Karypis' Clock Function                                  *
******************************************************************/
double gk_WClockSeconds(void) {
#ifdef __GNUC__
  struct timeval ctime;

  gettimeofday(&ctime, NULL);

  return (double)ctime.tv_sec + (double).000001*ctime.tv_usec;
#else
  return (double)time(NULL);
#endif
} //end gk_WClockSeconds



/******************************************************************
* Print the longest common substring from the c matrix           *
******************************************************************/
void LCS_print(short int **c, char *x, int i, int j) {
    if( i == 0 || j == 0 )
        return;
    else if( c[i][j]-1 == c[i-1][j-1] ) {
        LCS_print(c, x, i-1, j-1);
        printf("%c", x[i-1]);
    }
    else if( c[i][j] == c[i-1][j] )
        LCS_print(c, x, i-1, j);
    else
        LCS_print(c, x, i, j-1);

    return;
}



/******************************************************************
* Calculate the entries in a specific diagonal of the c matrix    *
******************************************************************/
void* LCS_diagonal(int k, LCS_t *LCS_info)
{

    int i,j;
    int start  = MAX(1,k-LCS_info->n);
    int finish = MIN(LCS_info->m,k-1);

    for( i = start; i <= finish; i++ )
    {
        j = k - i;

        /* Populate c[i][j] with appropriate value */
        if(LCS_info->x[i-1] == LCS_info->y[j-1])
            LCS_info->c[i][j] = LCS_info->c[i-1][j-1] + 1;
        else
        {
            if( LCS_info->c[i-1][j] >= LCS_info->c[i][j-1] )
                LCS_info->c[i][j] = LCS_info->c[i-1][j];
            else
                LCS_info->c[i][j] = LCS_info->c[i][j-1];
        }
        /* Populated */
    }
}

/******************************************************************
* Find the length of the Longest Common Substring(s) of x & y     *
******************************************************************/
void LCS_length(LCS_t *LCS_info, int myid, int num_procs )
{
    int diag, col;


//    printf("N: %d\n", LCS_info->n);
    /* For each diagonal */
    for(diag = 2; diag <= LCS_info->m+LCS_info->n; diag++)
    {
        /* Receive value of first row to myid+1 */
        if (myid != 0 && diag-1 <= LCS_info->n ) {
            MPI_Recv(&LCS_info->c[0][diag-1], 1, MPI_INT, myid-1, 0, MPI_COMM_WORLD, NULL);
 //           printf("Process %d m: %d\n", myid, LCS_info->m);
  //          printf("Process %d recv c[0][%d] = %d\n", myid, diag-1, LCS_info->c[0][diag-1]);
        }

        /* Populate diagonal k of process's c matrix */
        LCS_diagonal(diag, LCS_info);


        /* Send value last row to myid-1 */
        if (myid != num_procs-1 && diag > LCS_info->m ) {
            col = diag - LCS_info->m;
            MPI_Send(&LCS_info->c[LCS_info->m][col], 1, MPI_INT, myid+1, 0, MPI_COMM_WORLD);
//            printf("Process %d sent c[%d][%d] = %d\n", myid, LCS_info->m, col, LCS_info->c[LCS_info->m][col]);
        }
    }


    return;
}
