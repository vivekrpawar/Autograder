#!/bin/bash


cat utilization_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs CPU utilization" -X "Number of Clients" -Y "Average CPU utilization (in %)" -r 0.25> ./utilization_plot.png
