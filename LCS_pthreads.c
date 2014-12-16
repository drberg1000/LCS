#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))

int NUMTHREADS=1;

#define gk_clearwctimer(tmr) (tmr = 0.0) 
#define gk_startwctimer(tmr) (tmr -= gk_WClockSeconds())
#define gk_stopwctimer(tmr)  (tmr += gk_WClockSeconds())
#define gk_getwctimer(tmr)   (tmr)


/******************************************************************
* Structs containing LCS information and arguments for pthreads   *
******************************************************************/
typedef struct LCS_t
{
    char *x;
    char *y;
    short int **c;
    int   m;
    int   n;
} LCS_t;

typedef struct args_t 
{
    LCS_t *LCS_info;
    pthread_barrier_t *bar;
    int thread_id;
    int k;
} args_t;

double gk_WClockSeconds(void);
void LCS_length(LCS_t *);
void LCS_print(short int **c, char *x, int i, int j);
char* LCS_read(char *);



/******************************************************************
* Output the current state of c                                   *
******************************************************************/
void print_c_wait(short int **c, int m, int n) {
    int row, col;

    for (row = 0; row <= m; row++) {
        for (col = 0; col <= n; col++)
            printf("%7d ", c[row][col]);
        printf("\n");
    }
    printf("Press enter to continue.\n");
    getchar();

    return;
}



/****************************************************
* Main stub to time & call LCS methods              *
****************************************************/
int main(int argc, char** argv)
{
    
    if( argc != 4 )
    {
        printf("Usage: %s threads file1 file2\n", argv[0]);
        exit(0);
    }

    NUMTHREADS = atoi(argv[1]);

    double timer_total;
    double timer_length;
    double timer_print;
    gk_clearwctimer(timer_total);
    gk_clearwctimer(timer_length);
    gk_clearwctimer(timer_print);
    gk_startwctimer(timer_total);

    /* Read from File */
    LCS_t LCS_info;
    LCS_info.x = LCS_read(argv[2]);
    LCS_info.y = LCS_read(argv[3]);

    LCS_info.m = (int) strlen(LCS_info.x);
    LCS_info.n = (int) strlen(LCS_info.y);

    /* Create array of size m x n */
    int i;
    LCS_info.c = malloc(sizeof(int*)*(LCS_info.m+1));
    for( i = 0; i < LCS_info.m+1; i++)
        LCS_info.c[i] = malloc(sizeof(int)*(LCS_info.n+1));


    /* Find the length of the LCS */
    gk_startwctimer(timer_length);
    LCS_length(&LCS_info);                
    gk_stopwctimer(timer_length);


    /* Print the LCS */
    gk_startwctimer(timer_print);
    LCS_print(LCS_info.c, LCS_info.x, LCS_info.m, LCS_info.n);
    printf("\n");
    gk_stopwctimer(timer_print);


    /* Print Execution times & LCS */
    gk_stopwctimer(timer_total);
    printf("Number of threads: %d\n", NUMTHREADS);
    printf("Time Taken Overall      : %f sec\n", timer_total);
    printf("Time Taken by LCS_Length: %f sec\n", timer_length);
    printf("Time Taken by LCS_Print : %f sec\n", timer_print);
    printf("Length of LCS: %d\n", LCS_info.c[LCS_info.m][LCS_info.n]);

    return 0;

} //END MAIN

/****************************************************
*  Read a file into a char string ignoring newlines *
****************************************************/
char* LCS_read(char *infile) {
    FILE *FIN;
    char c = 'n';
    char *S = NULL;
    char *T;
    int count=0;

    if( (FIN = fopen(infile, "r")) == NULL ) {
        printf("Problem opening %s.\n", infile);
        exit(0);
    }

    while ( (c=fgetc(FIN)) != EOF ) {
        if( c != '\n' )  /* Ignore new lines & EOF */ {
            count++;

            T = (char *) realloc((void *) S, (count+1)*sizeof(char));

            if( T != NULL) {
                S = T;
                S[count-1] = c;
                S[count] = '\0';
            }
            else {
                free(S);
                printf("Error (re)allocating memory to store string in %s\n", infile);
                exit(1);
            }
        }
    }

    fclose(FIN);

    return S;
} //end LCS_read



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
void LCS_print(short int **c, char *x, int i, int j){
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



/******************************************************************
* Calculate the entries in a specific diagonal of the c matrix    *
******************************************************************/
void* LCS_diagonals( void *args )
{
    int i,j,k;
    args_t *a = (args_t*) args;
    int thread_id = (int) a->thread_id;
    LCS_t *LCS_info = a->LCS_info;

   
    int top_row, bot_row, nrows, start, finish;
    /* Step through matrix diagonally ignoring row 0 and column 0 */
    for(k = 2; k <= LCS_info->m+LCS_info->n; k++)
    {
        top_row = MAX(1,k-LCS_info->n);
        bot_row = MIN(LCS_info->m,k-1); 
        nrows  = ceil((double)(bot_row - top_row + 1)/NUMTHREADS);
        start  = top_row + nrows*thread_id;
        finish = MIN(bot_row, start+nrows-1);


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

        pthread_barrier_wait( a->bar );
    }
}

/******************************************************************
* Find the length of the Longest Common Substring(s) of x & y     *
******************************************************************/
void LCS_length(LCS_t *LCS_info) 
{
    args_t args[NUMTHREADS];
    pthread_barrier_t bar;
    pthread_barrier_init(&bar, NULL, NUMTHREADS);


    
    /* Zero out first column & row */
    int i,j,k;
    for( i = 0; i <= LCS_info->m; i++ ) 
        LCS_info->c[i][0] = 0;
    for( j = 1; j <= LCS_info->n; j++ )
        LCS_info->c[0][j] = 0;


    pthread_t thread[NUMTHREADS];
    for( i = 0 ; i < NUMTHREADS; i++ )
    {
        args[i].LCS_info = LCS_info;
        args[i].thread_id = i;
        args[i].bar = &bar;
    }


    for( i = 0 ; i < NUMTHREADS; i++ )
    {
        args[i].k = k;
        pthread_create( &thread[i], NULL, LCS_diagonals, (void *) &args[i]);
    }
    for( i = 0 ; i < NUMTHREADS; i++ )
        pthread_join(thread[i], NULL);
//        print_c_wait(LCS_info->c, LCS_info->m, LCS_info->n);


    return;
}
