#!/bin/bash

# Loop through each directory in the current directory
for dir in */; do
  # Check if the directory exists and is a directory
  if [ -d "$dir" ]; then
    # Change to the conn_check_files/ directory within the current directory
    if cd "$dir/conn_check_files/" 2>/dev/null; then
      # Initialize a flag to indicate if all files are non-empty
      all_non_empty=true

      # Check if files matching the patterns exist
      for file in *holes.txt *elect.txt; do
        if [ -e "$file" ]; then
          if [ ! -s "$file" ]; then
            all_non_empty=false
            break
          fi
        else
          all_non_empty=false
          break
        fi
      done

      # If all files are non-empty, proceed with the commands
      if $all_non_empty; then
        # List all files matching the patterns and save to a.txt
        ls *holes.txt *elect.txt > a.txt
        
        # Copy the analysis macro file
        cp ../../../analysis_macros/my_analysis/my_analysis_conn_check.C .
        
        # Copy the scan script
        cp ../../../bash_scripts/scan_z_alpha_n_neigh_ch.sh .
        
        # Set executable permissions for the scan script
        chmod a+x scan_z_alpha_n_neigh_ch.sh
        
        # Execute the scan script
        ./scan_z_alpha_n_neigh_ch.sh
      else
        echo "No matching non-empty files in ${dir%/}/conn_check_files/, skipping..."
      fi
      
      # Return to the original directory
      cd - > /dev/null || exit
    else
      echo "Cannot access ${dir%/}/conn_check_files/, skipping..."
    fi
  fi
done
