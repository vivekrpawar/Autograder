#! /usr/bin/bash 

if [ $# -ne 4 ]; then
	echo "Usage $0 <numClients> <loopNum> <sleepTimeSeconds> <timeout-seconds>"
	exit 1
fi 
arg2="$2"
arg3="$3"
arg4="$4" 

M=$1
x=1
total_throughput=0
total_error_rate=0
total_req_sent_rate=0
total_goodput=0
total_timeout_rate=0 
sum_N=0
sum_NR=0
rm -r loadtest-output
mkdir loadtest-output

 

port=8001

for (( i=1 ; i<=$M ; i++ )); 
do	
	echo "Launching client $i"
	filename="./loadtest-output/output$i.txt"  
	./submit 127.0.0.1:$port print-sequence.cpp $arg2 $arg3 $arg4> "$filename" &	 
done

touch utilization_result.txt
touch no_of_threads.txt 
total_threads=0
for (( i = 1; i <100; i++ ));
do
	# Calculating no of threads 
	num_threads=$(ps -eLf | grep "./server $port" | head -1 | awk '{print $6}')
	
	# Calculating total number of threads 
	total_threads=$(echo $total_threads+$num_threads | bc -l) 
done
wait 

for (( i=1 ; i<=$M ; i++ )); 
do
	filename="./loadtest-output/output$i.txt"  
	avg_res=$(cat $filename | grep "Average response time:" | cut -d ' ' -f 4)
	N_i=$(cat $filename | grep "Successful responses:" | cut -d ' ' -f 3)
	
	#Calculate the throughput and total throughput
	throughput=$(echo $x*1000/$avg_res | bc -l)
	total_throughput=$(echo $total_throughput+$throughput | bc -l)
	
	#Calculate the sum of N_i*R_i i.e. number_of_successful*average_response_time 
	sum_NR=$(echo $sum_NR+$avg_res*$N_i | bc -l)
	sum_N=$(echo $N_i+$sum_N | bc -l)	
	
	#Calculation sum important performance indicators
	#Reading data from the file
	error_rate=$(cat $filename | grep "Error rate:" | cut -d ' ' -f 3)
	req_sent_rate=$(cat $filename | grep "Request sent rate:" | cut -d ' ' -f 4)
	goodput=$(cat $filename | grep "Successful request rate:" | cut -d ' ' -f 4)
	timeout_rate=$(cat $filename | grep "Timeout rate:" | cut -d ' ' -f 3)
	
	#Adding all values
	total_error_rate=$(echo $total_error_rate+$error_rate | bc -l)
	total_req_sent_rate=$(echo $total_req_sent_rate+$req_sent_rate | bc -l)
	total_goodput=$(echo $total_goodput+$goodput | bc -l)
	total_timeout_rate=$(echo $total_timeout_rate+$timeout_rate | bc -l)
done
 
avg_no_of_threads=$(echo $total_threads/100| bc -l)
total_avg_response_time=$(echo $sum_NR/$sum_N | bc -l)
echo "Overall throughput: $total_throughput requests/sec" 
echo "Average response time: $total_avg_response_time"
echo "Total error rate: $total_error_rate"
echo "Successful request rate: $total_goodput"
echo "Total timeout rate: $total_timeout_rate" 
echo "Average no. of threads: $avg_no_of_threads"
