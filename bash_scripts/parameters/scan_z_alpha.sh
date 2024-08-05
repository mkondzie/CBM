#!/bin/bash

# List all files matching the pattern and save to a.txt
ls *holes.txt *elect.txt > a.txt

# Loop through the specified range for z_alpha_var
for z_alpha_var in $(seq 0.25 0.025 0.85); do
  # Use sed to modify line 252 in the my_analysis_conn_check.C file
  sed -i '252s|.*|  float z_alpha = '"$z_alpha_var"';|' my_analysis_conn_check.C
  
  # Run the root command with the modified script
  root -l -b -q my_analysis_conn_check.C
done
