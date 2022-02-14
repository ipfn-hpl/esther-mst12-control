#!/bin/bash
x=1
while [  $x -le 120 ]
do
#   echo -n '\x33aba\x0d\x0a'
   #echo "1, Stopped, Char:x, SwIN:0, SwOUT:0, LimIN:0, LimOUT:0, Millis:686343, Holding:0"
   echo "684"
   sleep 1
   x=$(( $x + 1 ))
done
