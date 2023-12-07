# /bin/bash
#
cat throughput_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Overall Throughtput vs No. of clients" -X "No. of Clients" -Y "Throughput (request/sec)" -r 0.25> ./throughputVsclients.png

cat avg_res_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Average response time vs No. of clients" -X "No. of Clients" -Y "Response Time (sec)" -r 0.25> ./avgrestimeVsclients.png


cat thread_num_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Average no. of threads vs No. of clients" -X "No. of Clients" -Y "Number of threads" -r 0.25> ./threadnumVsclients.png


cat cpu_utilization_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Average CPU utilization vs No. of clients" -X "No. of Clients" -Y "CPU utilization(%)" -r 0.25> ./cpuutilVsclients.png
