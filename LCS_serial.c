#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))

#define gk_clearwctimer(tmr) (tmr = 0.0) 
#define gk_startwctimer(tmr) (tmr -= gk_WClockSeconds())
#define gk_stopwctimer(tmr)  (tmr += gk_WClockSeconds())
#define gk_getwctimer(tmr)   (tmr)
double gk_WClockSeconds(void);

void LCS_length(char *x, char *y, int **c);
/*  LCS_Length calculates the length of the longest common substring shared 
    between the character strings pointed to by x and y.  The longest common
    subsequence between x_i and y_j is stored in the (m+1)x(n+1) matrix at 
    address c.  
    
    Here, x_i is first i characters of x, and y_j is the first j characters 
    of y.  x is m characters long and y is j characters long.  Thus,
    the LCS of x and y will be found at c[m][n]. */


void LCS_print(int **c, char *x, int i, int j);
/* LCS_print takes the matrix modified by LCS_length() and uses it, along 
   with recursive calls to itself to print a LCS of x_i and y_j to standard 
   out.  Thus calling LCS_print() with i=strlen(x) and j=strlen(y) will
   result in printing the LCS of x and y.

   NOTE: This algorithm will always print the same LCS, but it is not
         necessarily unique.   */

int main(int argc, char** argv)
{
    char *X = argv[1];
    char *Y = argv[2];

    double timer_length;
    double timer_print;
    int len_X = (int) strlen(X);
    int len_Y = (int) strlen(Y);
    int rowIt,colIt;
    int **c;
    
    /* Create matrix of sixe (len_X+1)x(len_Y+1) for LCS_*() */
    c = malloc(sizeof(int*) * (len_X+1));
    for( rowIt = 0; rowIt < len_X+1; rowIt++)
        c[rowIt] = malloc(sizeof(int)*(len_Y+1));


    gk_clearwctimer(timer_length);
    gk_startwctimer(timer_length);
    LCS_length(X, Y, c);
    gk_stopwctimer(timer_length);

    gk_clearwctimer(timer_print);
    gk_startwctimer(timer_print);
    LCS_print(c, X, len_X, len_Y);
    gk_stopwctimer(timer_print);


    printf("\n\n");
    printf("timer_print: %f\n", timer_print);
    printf("timer_length: %f\n", timer_length);

    /* Free rows of c and c itself */
    for( rowIt = 0; rowIt < len_X+1; rowIt++)
        free(c[rowIt]);
    free(c);


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


void LCS_length(char *x, char *y, int **c) 
{
    int rowIt, colIt, digIt;
    int len_X = (int) strlen(x);
    int len_Y = (int) strlen(y);

    /* Zero out first column & row */
    for( rowIt = 0; rowIt <= len_X; rowIt++ ) 
        c[rowIt][0] = 0;
    for( colIt = 1; colIt <= len_Y; colIt++ )
        c[0][colIt] = 0;

    
    /* Step through matrix diagonally ignoring row 0 and column 0 */
    /* Diagonal iteration complicates indices but allows a better
       parallel implementation */
    for(digIt = 2; digIt <= len_X+len_Y; digIt++) {
        for(rowIt =  MIN(len_X, digIt-1); 
            rowIt >= MAX(1    , digIt-len_Y); 
            rowIt--)
        {
            colIt = digIt - rowIt;

            /* Populate c[rowIt][colIt] with appropriate value */
            if(x[rowIt-1] == y[colIt-1]) {
                /* Strings match so, cell gets NorthWest neighbor + 1 */
                c[rowIt][colIt] = c[rowIt-1][colIt-1] + 1;
            }
            else {
                /* Cell gets max of North and West neighbor counts. */
                /* Tie goes to the North */
                c[rowIt][colIt] = MAX( c[rowIt-1][colIt  ],
                                       c[rowIt  ][colIt-1] );
            }
            /* Populated */
        }
    }

    return;
}



void LCS_print(int **c, char *x, int i, int j){
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
