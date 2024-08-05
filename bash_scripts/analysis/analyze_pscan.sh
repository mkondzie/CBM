#!/bin/bash

# Check if module_id is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <module_id>"
  exit 1
fi

module_id="$1"

# Navigate to the module's pscan_files directory
cd "$module_id/pscan_files/" || exit

# Copy the necessary files from the source directory
cp ../../../analysis_macros/20240419/execution.C .
cp ../../../analysis_macros/20240419/trim_adc.cxx .
cp ../../../analysis_macros/20240419/trim_adc.hxx .
cp ../../../analysis_macros/20240419/plot_1024.C .

# List the relevant pscan files and save to a.txt
ls pscan*elect.txt pscan*holes.txt > a.txt

# Run ROOT commands
root -l <<EOF
.L trim_adc.cxx
.L execution.C
execution()
.q
EOF

# List the generated ROOT files and save to plot.txt
ls pscan*elect.root pscan*holes.root > plot.txt

# Run the plotting macro
root -l plot_1024.C
