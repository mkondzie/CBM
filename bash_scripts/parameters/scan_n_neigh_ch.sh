#!/bin/bash

# List all files matching the pattern and save to a.txt
ls *holes.txt *elect.txt > a.txt

# Loop through the specified range for n_neigh_ch_var
for n_neigh_ch_var in $(seq 1 1 50); do
  # Use sed to modify line 252 in the my_analysis_conn_check.C file
  sed -i '254s|.*|  int n_neigh_ch = '"$n_neigh_ch_var"';|' my_analysis_conn_check.C
  
  # Run the root command with the modified script
  root -l -b -q my_analysis_conn_check.C
done
