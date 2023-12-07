# /bin/bash

if [ $# -ne 2 ];then
	echo "Usage: $1 <number of clients> <port>"
	exit 1	
fi
M=$1

port=$2
thread_log="./serv_analysis_dir/threads_dir/threads_log$M.txt"
cpu_log="./serv_analysis_dir/cpu_util_dir/utilization_log$M.txt"
rm $thread_log
touch $thread_log
rm $cpu_log
touch $cpu_log
for (( ;; ));
do
	# Calculating no of threads 
	num_threads=$(ps -eLf | grep "./gradingserver $port" | head -1 | awk '{print $6}')
	echo $num_threads >> $thread_log
	
	# Calculating cpu utilization
	cpu_id=$(vmstat | awk '{print $15}' | tail -1)
	cpu_util=$(echo 100-$cpu_id | bc -l) 
	echo $cpu_util >> $cpu_log
	sleep 0.5
done


