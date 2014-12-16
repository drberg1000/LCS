#include <omp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))

int NUMTHREADS=1;

#define gk_clearwctimer(tmr) (tmr = 0.0)
#define gk_startwctimer(tmr) (tmr -= gk_WClockSeconds())
#define gk_stopwctimer(tmr)  (tmr += gk_WClockSeconds())
#define gk_getwctimer(tmr)   (tmr)
double gk_WClockSeconds(void);

void LCS_length(char *x, char *y, unsigned short int **c);
/*  LCS_Length calculates the length of the longest common substring shared
    between the character strings pointed to by x and y.  The longest common
    subsequence between x_i and y_j is stored in the (m+1)x(n+1) matrix at
    address c.

    Here, x_i is first i characters of x, and y_j is the first j characters
    of y.  x is m characters long and y is j characters long.  Thus,
    the LCS of x and y will be found at c[m][n]. */


void LCS_print(unsigned short int **c, char *x, int i, int j);
/* LCS_print takes the matrix modified by LCS_length() and uses it, along
   with recursive calls to itself to print a LCS of x_i and y_j to standard
   out.  Thus calling LCS_print() with i=strlen(x) and j=strlen(y) will
   result in printing the LCS of x and y.

   NOTE: This algorithm will always print the same LCS, but it is not
         necessarily unique.   */


char* LCS_read(char *infile);
/* Return character string read from file pointed to by infile.
   Newlines are ignored. */


double main(int argc, char** argv)
{

    if( argc != 4 )
    {
        printf("Usage: %s threads file1 file2\n", argv[0]);
        exit(0);
    }

    NUMTHREADS = atoi(argv[1]);

    double timer_total;
    gk_clearwctimer(timer_total);
    gk_startwctimer(timer_total);

    /* Read from File */
    double timer_read;
    gk_clearwctimer(timer_read);
    gk_startwctimer(timer_read);
    char *X = LCS_read(argv[2]);
    char *Y = LCS_read(argv[3]);
    gk_stopwctimer(timer_read);

    int len_X = (int) strlen(X);
    int len_Y = (int) strlen(Y);

    if( len_X+len_Y + len_X*8 + (len_X*len_Y)*2 > 4294967296 /* 4*2^30 */ )
    {
        printf("Error\n");
        printf("Strings are too large for this implementation with 4G ram.  Aborting.\n");
        return 0;
    }

    /* Create matrix of size (len_X+1)x(len_Y+1) for LCS_*() */
    /* Above limit ==> length of LCS < sqrt(2^31) < 2^16-1 == USHRT_MAX */
    /* Therefore, unsigned short ints can be used for c.  */
    unsigned short int **c;
    int rowIt;
    c = malloc(sizeof(unsigned short int*) * (len_X+1));
    for( rowIt = 0; rowIt < len_X+1; rowIt++)
        c[rowIt] = malloc(sizeof(unsigned short int)*(len_Y+1));


    double timer_length;
    gk_clearwctimer(timer_length);
    gk_startwctimer(timer_length);
    LCS_length(X, Y, c);
    gk_stopwctimer(timer_length);

    double timer_print;
    gk_clearwctimer(timer_print);
    gk_startwctimer(timer_print);
    LCS_print(c, X, len_X, len_Y);
    printf("\n\n");
    gk_stopwctimer(timer_print);

    gk_stopwctimer(timer_total);
    printf("Number of threads: %d\n", NUMTHREADS);
    printf("Time Taken Overall      : %f sec\n", timer_total);
    printf("Time Taken by LCS_read: %f sec\n", timer_read);
    printf("Time Taken by LCS_Length: %f sec\n", timer_length);
    printf("Time Taken by LCS_Print : %f sec\n", timer_print);
    printf("Length of LCS: %d\n", c[len_X][len_Y]);

    /* Free rows of c and c itself */
    for( rowIt = 0; rowIt < len_X+1; rowIt++)
        free(c[rowIt]);
    free(c);
    free(X);
    free(Y);

    return timer_total;

}

/****************************************************
*  Read a file into a char string ignoring newlines *
****************************************************/
char* LCS_read(char *infile)
{
    FILE *FIN;
    char c = 'n';
    char *S = NULL;
    char *T;
    int count=0;

    if( (FIN = fopen(infile, "r")) == NULL )
    {
        printf("Problem opening %s.\n", infile);
        exit(0);
    }

    while ( (c=fgetc(FIN)) != EOF )
    {
        if( c != '\n' )  /* Ignore new lines & EOF */
        {
            count++;

            T = (char *) realloc((void *) S, (count+1)*sizeof(char));

            if( T != NULL)
            {
                S = T;
                S[count-1] = c;
                S[count] = '\0';
            }
            else
            {
                free(S);
                printf("Error (re)allocating memory to store string in %s\n", infile);
                exit(1);
            }
        }
    }

    fclose(FIN);

    return S;
}


void print_C_pause(short unsigned int **c, int len_X, int len_Y)
{
    int row;
    int col;

    for (row = 0; row <= len_X; row++)
    {
        for (col = 0; col <= len_Y; col++)
            printf("%3d ", c[row][col]);
        printf("\n");
    }

    printf("Press any key to continue.\n");
    getchar();

    return;
}


double gk_WClockSeconds(void)
{
#ifdef __GNUC__
  struct timeval ctime;

  gettimeofday(&ctime, NULL);

  return (double)ctime.tv_sec + (double).000001*ctime.tv_usec;
#else
  return (double)time(NULL);
#endif
}


void LCS_length(char *x, char *y, unsigned short int **c)
{
    int rowIt, colIt;
    int len_X = (int) strlen(x);
    int len_Y = (int) strlen(y);

    /* Zero out first column & row */
    for( rowIt = 0; rowIt <= len_X; rowIt++ )
        c[rowIt][0] = 0;
    for( colIt = 1; colIt <= len_Y; colIt++ )
        c[0][colIt] = 0;

#pragma omp parallel default(none) \
                         shared(x, y, c, len_X, len_Y, NUMTHREADS) \
                         private(rowIt,colIt) \
                         num_threads(NUMTHREADS)
    {
        int mythread = omp_get_thread_num();
        int row;
        int col;
        for(rowIt = 1; rowIt <= len_X; rowIt+=NUMTHREADS)         /* Work with rows rowIt+NUMTHREADS */
        {
            row = rowIt + mythread;
            if( row <= len_X )
            {
                for( colIt = 1; colIt <= len_Y+NUMTHREADS; colIt++  )
                {
                    if( (colIt - mythread >= 0) && (colIt - mythread <= len_Y) ) /* Stagger threads */
                    {
                        col = colIt - mythread;
                        /* Populate c[row][col] with appropriate value */
                        if(x[row-1] == y[col-1])
                            /* Strings match so, cell gets NorthWest neighbor + 1 */
                            c[row][col] = c[row-1][col-1] + 1;
                        else {
                            /* Cell gets max of North and West neighbor counts. */
                            /* Tie goes to the North */
                            c[row][col] = MAX( c[row-1][col  ], 
                                               c[row  ][col-1] );
                        }/* Populated */
#pragma omp critical
                        {
                            printf("My thread: %d\n",  mythread);
                            print_C_pause(c,len_X,len_Y);
                        }
                    }
                }
            }
        }
    }
    return;
}


/******************************************************************
* Create the longest common substring from the c matrix           *
******************************************************************/
void LCS_print(unsigned short int **c, char *x, int i, int j){
    if( i == 0 || j == 0 )
        return;

    if( (c[i][j]-1 == c[i-1][j-1])  &&  //Cell == NW neighbor + 1
        (c[i][j]-1 == c[i-1][j  ])  &&  //Cell == W  neighbor + 1
        (c[i][j]-1 == c[i  ][j-1])  )   //Cell == N  neighbor + 1
    {
        /* Cell corresponds to a match. */
        /* Construct rest of substring. */
        LCS_print(c, x, i-1, j-1);
        /* Append character from x corresponding to position (i,j). */
        /* Because c is (len_X+1)x(len_Y+1) this is c[i-1].  */
        printf("%c", x[i-1]);
    }
    else if( c[i][j] == c[i-1][j] )   //Go W if count is equal
        LCS_print(c, x, i-1, j);
    else
        LCS_print(c, x, i, j-1);      //W is less than N.  Go N.

    return;
}
