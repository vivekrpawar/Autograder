#!/bin/bash
echo Analysing and Generating Plots!!

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
'
dir=./Performance-results 
rm $dir/throughput_results.txt
rm $dir/avg_res_results.txt
rm $dir/error_rate_results.txt
rm $dir/goodput_results.txt
rm $dir/timeout_rate_results.txt 
rm $dir/thread_num_results.txt
rm $dir/avg_queue_len_results.txt

touch $dir/throughput_results.txt
touch $dir/avg_res_results.txt
touch $dir/error_rate_results.txt
touch $dir/goodput_results.txt
touch $dir/timeout_rate_results.txt 
touch $dir/thread_num_results.txt
touch $dir/avg_queue_len_results.txt

for i in ${SIZE}; do
    echo "Running $i clients"
    echo -n "${i} " >> $dir/throughput_results.txt
    echo -n "${i} " >> $dir/avg_res_results.txt
    echo -n "${i} " >> $dir/error_rate_results.txt
    echo -n "${i} " >> $dir/goodput_results.txt
    echo -n "${i} " >> $dir/timeout_rate_results.txt 
    echo -n "${i} " >> $dir/thread_num_results.txt
    echo -n "${i} " >> $dir/avg_queue_len_results.txt
    ./loadtest.sh ${i} 5 2 20 > tempoutput.txt 
    cat tempoutput.txt | grep "Overall throughput:" | cut -d ' ' -f 3 >> $dir/throughput_results.txt
    cat tempoutput.txt | grep "Average response time:" | cut -d ' ' -f 4 >> $dir/avg_res_results.txt
    cat tempoutput.txt | grep "Total error rate:" | cut -d ' ' -f 4 >> $dir/error_rate_results.txt
    cat tempoutput.txt | grep "Successful request rate:" | cut -d ' ' -f 4 >> $dir/goodput_results.txt
    cat tempoutput.txt | grep "Total timeout rate:" | cut -d ' ' -f 4 >> $dir/timeout_rate_results.txt 
    cat tempoutput.txt | grep "Average no. of threads:" | cut -d ' ' -f 5 >> $dir/thread_num_results.txt
    cat tempoutput.txt | grep "Average number of requests in the queue:" | cut -d ' ' -f 8 >> $dir/avg_queue_len_results.txt
done
rm tempoutput.txt

plotsdir=./Performance-Plots
# Plot Number of clients vs Overall throughput 
cat $dir/throughput_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Throughput" -X "Number of Clients" -Y "Overall throughput (in request/s)" -r 0.25> $plotsdir/throughput_plot.png

# Plot Number of clients vs Average response time 
cat $dir/avg_res_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Response Time" -X "Number of Clients" -Y "Average response time (in ms)" -r 0.25> $plotsdir/avg_res_plot.png

cat $dir/error_rate_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L  "Clients vs Error rate" -X "Number of Clients" -Y "Overall error rate (in request/s)" -r 0.25> $plotsdir/error_rate_plot.png

cat $dir/goodput_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Successful request rate" -X "Number of Clients" -Y "Successful request rate (in request/s)" -r 0.25> $plotsdir/successful_req_rate_plot.png

cat $dir/thread_num_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Timeout rate" -X "Number of Clients" -Y "Average no. of threads (in threads/client)" -r 0.25> $plotsdir/threads_plot.png

cat $dir/avg_queue_len_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Requests in Queue" -X "Number of Clients" -Y "Average no. of requests in Queue" -r 0.25> $plotsdir/queued_requests_plot.png

