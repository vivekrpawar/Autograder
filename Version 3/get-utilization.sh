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
32
'

port=8080
rm -r utilization-outputs
mkdir utilization-outputs
for M in ${SIZE}; do
	echo "Running $M clients"
	vmstat 1 > ./vmstat-output/vmstat-output-$M-.txt & 
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


# Define an array of patterns
patterns=("procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----" " r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st")

# Specify the input file
input_file="input.txt"

# Specify the output file
output_file="output.txt"

# Remove lines matching the specified patterns
for input_file in ./vmstat-output/*;
do
	for pattern in "${patterns[@]}"; do
    		sed -i "/$pattern/d" "$input_file"	# Remove the lines matching pattern
	done
	sed -i '1d;$d' "$input_file"	# Remove the first and last lines
done

# Remove the first and last lines
sed -i '1d;$d' "$input_file"

echo "Removed lines matching the specified patterns, as well as the first and last lines."

rm utilization_results.txt
touch utilization_results.txt

for i in ${SIZE};
do 
	
    	echo -n "${i} " >> ./Performance-results/utilization_results.txt
	# Calculate the average of the 2nd column
	average_idle_percentage=$(awk '{sum+=$15} END{print sum/NR}' ./vmstat-output/vmstat-output-$i-.txt)
	average_utilization=$(echo 100-$average_idle_percentage | bc -l)
	echo $average_utilization >> ./Performance-results/utilization_results.txt
done


rm ./output-dir/*output.txt
rm ./submission-dir/*program.cpp
rm ./comparison-dir/*comparison.txt
rm ./run-error-dir/*run-error.txt
rm ./comp-error-dir/*comp-error.txt
rm ./output-dir/*output.txt
rm ./executable-dir/*program.o