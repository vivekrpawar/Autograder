#!/bin/bash
echo Analysing and Generating Plots!!

# Different number of clients
SIZE='
1
2
3
4
5
6
7
8
9
10
'
rm -r analysis_results
mkdir analysis_results

throughput_results="./analysis_results/throughput_results.txt"
avg_res_results="./analysis_results/avg_res_results.txt"
error_rate_results="./analysis_results/error_rate_results.txt"
timeout_rate_results="./analysis_results/timeout_rate_results.txt" 
thread_num_results="./analysis_results/thread_num_results.txt"  
cpu_utilization_results="./analysis_results/cpu_utilization_results.txt"
tempoutput="./analysis_results/tempoutput.txt"

touch $throughput_results
touch $avg_res_results 
touch $thread_num_results
touch $cpu_utilization_results

for i in ${SIZE}; do
    echo "launching $i clients"
    echo -n "${i} " >> $throughput_results
    echo -n "${i} " >> $avg_res_results 
    echo -n "${i} " >> $thread_num_results
    echo -n "${i} " >> $cpu_utilization_results
    ./loadtest.sh ${i} 5 5 50 > $tempoutput 
    cat $tempoutput | grep "Overall throughput:" | cut -d ' ' -f 3 >> $throughput_results
    cat $tempoutput | grep "Average response time:" | cut -d ' ' -f 4 >> $avg_res_results 
    cat $tempoutput | grep "Average number of threads:" | cut -d ' ' -f 5 >> $thread_num_results 
    cat $tempoutput | grep "Average cpu utilization:" | cut -d ' ' -f 4 >> $cpu_utilization_results
done
