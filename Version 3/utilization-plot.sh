#!/bin/bash


cat ./Performance-results/utilization_results.txt | graph -T png --bitmap-size "1400x1400" -g 3 -L "Clients vs CPU utilization" -X "Number of Clients" -Y "Average CPU utilization (in %)" -r 0.25> ./Performance-Plots/utilization_plot.png
