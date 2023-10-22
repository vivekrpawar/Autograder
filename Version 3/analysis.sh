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
22
24
26
28
30
'
rm throughput_results.txt
rm avg_res_results.txt
rm error_rate_results.txt
rm goodput_results.txt
rm timeout_rate_results.txt 
rm thread_num_results.txt

touch throughput_results.txt
touch avg_res_results.txt
touch error_rate_results.txt
touch goodput_results.txt
touch timeout_rate_results.txt 
touch thread_num_results.txt

for i in ${SIZE}; do
    echo -n "${i} " >> throughput_results.txt
    echo -n "${i} " >> avg_res_results.txt
    echo -n "${i} " >> error_rate_results.txt
    echo -n "${i} " >> goodput_results.txt
    echo -n "${i} " >> timeout_rate_results.txt 
    echo -n "${i} " >> thread_num_results.txt
    ./loadtest.sh ${i} 5 2 10 > tempoutput.txt 
    cat tempoutput.txt | grep "Overall throughput:" | cut -d ' ' -f 3 >> throughput_results.txt
    cat tempoutput.txt | grep "Average response time:" | cut -d ' ' -f 4 >> avg_res_results.txt
    cat tempoutput.txt | grep "Total error rate:" | cut -d ' ' -f 4 >> error_rate_results.txt
    cat tempoutput.txt | grep "Successful request rate:" | cut -d ' ' -f 4 >> goodput_results.txt
    cat tempoutput.txt | grep "Total timeout rate:" | cut -d ' ' -f 4 >> timeout_rate_results.txt 
    cat tempoutput.txt | grep "Average no. of threads:" | cut -d ' ' -f 5 >> thread_num_results.txt
done
rm tempoutput.txt
# Plot Number of clients vs Overall throughput 
cat throughput_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Throughput" -X "Number of Clients" -Y "Overall throughput (in request/s)" -r 0.25> ./throughput_plot.png

# Plot Number of clients vs Average response time 
cat avg_res_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Response Time" -X "Number of Clients" -Y "Average response time (in ms)" -r 0.25> ./avg_res_plot.png

cat error_rate_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L  "Clients vs Error rate" -X "Number of Clients" -Y "Overall error rate (in request/s)" -r 0.25> ./error_rate_plot.png

cat goodput_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Successful request rate" -X "Number of Clients" -Y "Successful request rate (in request/s)" -r 0.25> ./successful_req_rate_plot.png

cat thread_num_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs Timeout rate" -X "Number of Clients" -Y "Average no. of threads (in threads/client)" -r 0.25> ./threads_plot.png

