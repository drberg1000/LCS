#String Description
#gcc LCS_serial.c     -o LCS_serial
gcc -fopenmp LCS_omp_diagonal.c -o LCS_omp_diagonal

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
#   RESULT=$(./LCS_serial ${S1[$i]} ${S2[$i]} | head -1 )
   RESULT=$(./LCS_omp_diagonal 2 ${S1[$i]} ${S2[$i]} | head -1 )

   if [ "$RESULT" = `cat "${S[$i]}"` ] 
   then
     success="PASSED" 
   else
     success="FAILED"
   fi
   printf "%-20s: [$success]\n" "${N[$i]}"
done
