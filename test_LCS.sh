#String Description
gcc LCS_serial.c -o LCS_serial
N=(  Tiny\ Strings\ from\ CLRS
     Long\ Strings\ from\ CLRS
  )

##String 1
S1=(  ABCBDAB
     ACCGGTCGAGTGCGCGGAAGCCGGCCGAA
  )

#String 2
S2=( BDCABA
    GTCGTTCGGAATGCCGTTGCTCTGTAAA
  )

#Longest Common Substring
S=( BCBA
    GTCGTCGGAAGCCGGCCGAA
  )



for ((i=0; i < ${#S1[@]}; i++));
do
   RESULT=$(./LCS_serial ${S1[$i]} ${S2[$i]} | head -1 )

   if [ "$RESULT" = "${S[$i]}" ] 
   then
     success="PASSED" 
   else
     success="FAILED"
   fi
   printf "%-20s: [$success]\n" "${N[$i]}"
done
