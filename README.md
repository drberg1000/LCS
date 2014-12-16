# Longest Common Substring

This repository currently several implementations of LCS.

- LCS\_serial\_row: runs on one core and fills the matrix row wise.
- LCS\_serial: runs on one core and fills the matrix in diagonal order.
- LCS\_omp\_diagonal: accepts a anumber of cores as an argument and uses 
     OpenMP for threading.  Otherwise, it's identical to LCS_serial.
- LCS\_mpi: uses the Open Message Passing Interface to create an LCS routine
     that will run on multiple computers in a cluster as well on a multi
     core machine. **NOTE** The 12/16/2014 version will not print an LCS.  It
     only finds it's length. As a result, LCS_mpi fails the test_LCS.sh check.
      **TODO** Review MPI function calls, clean up code, and replace LCS_Print 
      with a distributed serial format (ie. each process returns/prints just 
      it's portion of the LCS.
- LCS\_pthreads: Threading with pthreads correctly returns the LCS length and
      the same LCS as LCS_serial* and LCS_omp. This version could also use some
      clean up. 

## test\_LCS 
compiles and verifies functionality against micro\_test\*.txt and 
tiny\_test\*.txt.  If the argument correspond to versions as follows:
- serial   == diagonal wise serial
- row      ==      row wise serial
- diagonal == diagonal wise omp

The serial & OpenMP implementations use: 
    len_X+len_Y + len_X*8 + (len_X*len_Y)*2

bytes of memory on the heap.  On machine with 4G of RAM two strings of ~46,341 
bytes will cause paging.  The code checks to make sure the above formula is 
under 4\*2^30.  If the strings are longer than this, the program will abort.
The mpi and pthreads likely use more memory than this but don't yet have the
same check.  With MPI having the ability to run on multiple machines it's
it's limits should consider this and be conditionally raised.



## Main routines from serial and OpenMP versions.  
### void LCS\_length(char \*x, char \*y, int \*\*c);
LCS\_Length calculates the length of the longest common substring shared 
    between the character strings pointed to by x and y.  The longest common
    subsequence between x\_i and y\_j is stored in the (m+1)x(n+1) matrix at 
    address c.  
    
Here, x\_i is first i characters of x, and y\_j is the first j characters 
    of y.  x is m characters long and y is j characters long.  Thus,
    the LCS of x and y will be found at c[m][n].


### void LCS\_print(int \*\*c, char \*x, int i, int j);
LCS\_print takes the matrix modified by LCS\_length() and uses it, along 
  with recursive calls to itself to print a LCS of x\_i and y\_j to standard 
  out.  Thus calling LCS\_print() with i=strlen(x) and j=strlen(y) will
  result in printing the LCS of x and y.

NOTE: This algorithm will always print the same LCS, but it is not
         necessarily unique.  
