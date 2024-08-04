#!/bin/bash

# List all files matching the pattern and save to a.txt
ls *holes.txt *elect.txt > a.txt

# Loop through the specified range for n_neigh_ch_var and z_alpha_var
for n_neigh_ch_var in $(seq 5 5 40); do
  for z_alpha_var in $(seq 0.25 0.05 0.70); do
    # Modify line 254 for n_neigh_ch_var
    sed -i '254s|.*|  int n_neigh_ch = '"$n_neigh_ch_var"';|' my_analysis_conn_check.C
    
    # Modify line 252 for z_alpha_var
    sed -i '252s|.*|  float z_alpha = '"$z_alpha_var"';|' my_analysis_conn_check.C
    
    # Run the root command with the modified script
    root -l -b -q my_analysis_conn_check.C
  done
done
