#!/bin/bash

#Compile each version
gcc          LCS_serial_row.c   -o LCS_serial_row
gcc          LCS_serial_diag.c  -o LCS_serial_diag
gcc -fopenmp LCS_omp.c          -o LCS_omp
gcc          LCS_pthreads.c     -o LCS_pthreads     -lpthread -lm
mpicc        LCS_mpi.c          -o LCS_mpi

#Note, on my Ubuntu system I needed to install the packages openmpi-bin &
#  mpi-default-dev.  Then I was able to compile with mpicc and run with mpirun.

#String Description
N=(  Tiny\ Strings\ from\ CLRS
     Long\ Strings\ from\ CLRS
  )

##String 1
S1=( tests/micro_test1.txt
     tests/tiny_test1.txt
  )

#String 2
S2=( tests/micro_test2.txt
     tests/tiny_test2.txt
  )

#Longest Common Substring
S=( tests/micro_test_LCS.txt
    tests/tiny_test_LCS.txt
  )


for ((i=0; i < ${#S1[@]}; i++));
do
   if [ "$1" = "diagonal" ]
   then
      RESULT=$(./LCS_serial_diag ${S1[$i]} ${S2[$i]} | head -1 )
   elif [ "$1" = "row" ]
   then
      RESULT=$(./LCS_serial_row  ${S1[$i]} ${S2[$i]} | head -1 )
   elif [ "$1" = "omp" ]
   then
      RESULT=$(./LCS_omp 4 ${S1[$i]} ${S2[$i]} | head -1 )
   elif [ "$1" = "mpi" ]
   then
      RESULT=$(mpirun -np 4 LCS_mpi ${S1[$i]} ${S2[$i]} | head -1 )
   elif [ "$1" = "pthreads" ]
   then
      RESULT=$(./LCS_pthreads 4 ${S1[$i]} ${S2[$i]} | head -1 )
   else
      echo "Bad Argument"
      exit
   fi

   if [ "$RESULT" = `cat "${S[$i]}"` ] 
   then
     success="PASSED" 
   else
     success="FAILED"
   fi
   printf "%-20s: [$success]\n" "${N[$i]}"
done
