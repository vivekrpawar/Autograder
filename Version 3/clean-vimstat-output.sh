#!/bin/bash

# Define an array of patterns
patterns=("procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----" " r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st" "pattern3")

# Specify the input file
input_file="input.txt"

# Specify the output file
output_file="output.txt"

# Remove lines matching the specified patterns
for input_file in ./vmstat-output/*;
do
	for pattern in "${patterns[@]}"; do
    		sed -i "/$pattern/d" "$input_file"
	done
done

# Remove the first and last lines
sed -i '1d;$d' "$input_file"

# Copy the modified input file to the output file
#cp "$input_file" "$output_file"

echo "Lines matching the specified patterns, as well as the first and last lines."

