#!/bin/bash 

# Different number of clients
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

rm utilization_results.txt
touch utilization_results.txt

for i in ${SIZE};
do 
	
    	echo -n "${i} " >> utilization_results.txt
	# Calculate the average of the 2nd column
	average_idle_percentage=$(awk '{sum+=$15} END{print sum/NR}' ./vmstat-output/vmstat-output-$i-.txt)
	average_utilization=$(echo 100-$average_idle_percentage | bc -l)
	echo $average_utilization >> utilization_results.txt
done

