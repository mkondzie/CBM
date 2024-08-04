#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <NUMBER-OF-THE-SETUP>"
    exit 1
fi

setup_number=$1
filename="tester_setup${setup_number}_arr.py"

# Check if the file exists
if [ ! -f "$filename" ]; then
    echo "File not found: $filename"
    exit 1
fi

# Use sed to replace specific lines based on their content
sed -i 's|test_list_check = \["set_trim_calib", "#check_trim", "#turn_hv_off", "conn_check"\]|test_list_check = \["set_trim_calib", "check_trim", "#turn_hv_off", "#conn_check"\]|' "$filename"
sed -i 's|#test_list.extend(test_list_calib)|test_list.extend(test_list_calib)|' "$filename"

echo "Lines have been replaced for calibration in $filename"
