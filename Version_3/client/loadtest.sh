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
port=8080

./serverutils.sh $M $port &
serv_util_pid=$! 

pids=""
for (( i=1 ; i<=$M ; i++ )); 
do	 
	filename="./client_output_dir/client_output$i.txt"
	./gradingclient 127.0.0.1 $port program_runs.cpp $2 $3 $4 1>$filename &
	pids+="$! "
done 

for pid in $pids;do 
	wait $pid
done 

kill -9 $serv_util_pid 2>&1 > /dev/null

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

thread_log="./serv_analysis_dir/threads_dir/threads_log$M.txt"
cpu_log="./serv_analysis_dir/cpu_util_dir/utilization_log$M.txt"

threads=$(cat $thread_log) 
total_threads=0
count=0
for t in $threads;do
	count=$(echo $count+1 | bc -l)
	total_threads=$(echo $total_threads+$t | bc -l)
done

cpu_util=$(cat $cpu_log)
ucount=0
total_util=0
for u in $cpu_util;do
	ucount=$(echo $ucount+1 | bc -l)
	total_util=$(echo $total_util+$u | bc -l)
done

avg_no_threads=$(echo $total_threads/$count | bc -l)
total_avg_response_time=$(echo $sum_NR/$sum_N/ 1000 | bc -l)
average_cpu_utilization=$(echo $total_util/$ucount | bc -l)

echo "Overall throughput: $total_throughput requests/sec" 
echo "Average response time: $total_avg_response_time"
echo "Average number of threads: $avg_no_threads"
echo "Average cpu utilization: $average_cpu_utilization"
