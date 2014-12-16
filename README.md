# Longest Common Substring

This repository currently contains two implementaion of LCS.

- LCS\_serial: runs on one core and fills the matrix in diagonal order.
- LCS\_omp\_diagonal: accepts a anumber of cores as an argument and uses 
     OpenMP for threading.  Otherwise, it's identical to LCS_serial.

test_LCS compiles and verifies functionality against micro\_test\*.txt and 
tiny\_test\*.txt.  If the argument serial is passed, it tests the serial version.  
Otherwise the omp\_diagonal version is checked.

This implementation uses: 
    len_X+len_Y + len_X*8 + (len_X*len_Y)*2
bytes of memory on the heap.  On machine with 4G of RAM two strings of ~46,341 
bytes will cause paging.  The code checks to make sure the above formula is 
under 4\*2^30.  If the strings are longer than this, the program will abort.



## void LCS\_length(char \*x, char \*y, int \*\*c);
LCS\_Length calculates the length of the longest common substring shared 
    between the character strings pointed to by x and y.  The longest common
    subsequence between x\_i and y\_j is stored in the (m+1)x(n+1) matrix at 
    address c.  
    
Here, x\_i is first i characters of x, and y\_j is the first j characters 
    of y.  x is m characters long and y is j characters long.  Thus,
    the LCS of x and y will be found at c[m][n].


## void LCS\_print(int \*\*c, char \*x, int i, int j);
LCS\_print takes the matrix modified by LCS\_length() and uses it, along 
  with recursive calls to itself to print a LCS of x\_i and y\_j to standard 
  out.  Thus calling LCS\_print() with i=strlen(x) and j=strlen(y) will
  result in printing the LCS of x and y.

NOTE: This algorithm will always print the same LCS, but it is not
         necessarily unique.  


##LCS\_test.sh
Compiles LCS\_serial.c and tests it agains strings defined in the script.
