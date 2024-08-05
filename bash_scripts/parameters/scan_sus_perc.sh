#!/bin/bash

# List all files matching the pattern and save to a.txt
ls *holes.txt *elect.txt > a.txt

# Loop through the specified range for sus_perc_var
for sus_perc_var in $(seq 0.05 0.025 0.40); do
  # Use sed to modify line 252 in the my_analysis_conn_check.C file
  sed -i '256s|.*|  float sus_perc = '"$sus_perc_var"';|' my_analysis_conn_check.C
  
  # Run the root command with the modified script
  root -l -b -q my_analysis_conn_check.C
done
