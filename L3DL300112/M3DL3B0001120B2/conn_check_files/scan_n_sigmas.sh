#!/bin/bash

# List all files matching the pattern and save to a.txt
ls *holes.txt *elect.txt > a.txt

# Loop through the specified range for n_sigmas_var
for n_sigmas_var in $(seq 0.5 0.05 6.00); do
  # Use sed to modify line 252 in the my_analysis_conn_check.C file
  sed -i '259s|.*|  float n_sigmas = '"$n_sigmas_var"';|' my_analysis_conn_check.C
  
  # Run the root command with the modified script
  root -l -b -q my_analysis_conn_check.C
done
