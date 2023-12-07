#! /usr/bin/bash 

if [ $# -ne 4 ]; then
	echo "Usage $0 <numClients> <loopNum> <sleepTimeSeconds> <timeout-seconds>"
	exit 1
fi

rm -r client_output_dir
mkdir client_output_dir
M=$1
x=1
total_throughput=0
sum_N=0
sum_NR=0
for (( i=1 ; i<=$M ; i++ )); 
do	 
	filename="./client_output_dir/client_output$i.txt"
	./submit 127.0.0.1 8080 program_runs.cpp $2 $3 $4 1>$filename &
done
wait

for (( i=1 ; i<=$M ; i++ ));
do
	filename="./client_output_dir/client_output$i.txt"
	avg_res=$(cat $filename | grep "Average response time:" | cut -d ' ' -f 4)
	N_i=$(cat $filename | grep "Successful responses:" | cut -d ' ' -f 3)
	
	#Calculate the throughput and total throughput
	throughput=$(grep "Throughput:" $filename | cut -d ' ' -f 2)
	total_throughput=$(echo $total_throughput + $throughput | bc -l)
	
	#Calculate the sum of N_i*R_i i.e. number_of_successful*average_response_time 
	sum_NR=$(echo $sum_NR+$avg_res*$N_i | bc -l)
	
	# Sum of number of successful responses
	sum_N=$(echo $N_i + $sum_N | bc -l)	
done

total_avg_response_time=$(echo $sum_NR/$sum_N/ 1000| bc -l)
echo "Overall throughput: $total_throughput requests/sec" 
echo "Average response time: $total_avg_response_time"
