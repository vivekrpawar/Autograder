#! /usr/bin/bash 

SIZE='
2
4
6
8
10
12
14
16
18
20
22
24
26
28
30
'

port=8080
rm -r utilization-outputs
mkdir utilization-outputs
for M in ${SIZE}; do
	echo "Running $M clients"
	vmstat 1 > vmstat-output/vmstat-output-$M-.txt & 
	for (( i=1 ; i<=$M ; i++ )); do
		echo "Request sent no. $i"
		./submit 127.0.0.1:$port print-sequence.cpp 5 2 10 > tempoutput.txt &
		pids[${i}]=$!
	done
	
	for pid in ${pids[*]};do
		wait $pid
	done
	killall vmstat
done 
