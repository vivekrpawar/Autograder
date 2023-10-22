#! /bin/bash

touch utilization_result.txt
touch no_of_threads.txt
for i in {1..5}; do
	echo -n "${i} "
	vmstat | tail -1 | cut -d ' ' -f 32 >> utilization_result.txt
done
	
	

