#!/bin/bash

filename="feb_test.py"

# Use sed to replace the line containing test_list_check = 
sed -i '/active_downlinks = [1/s/.*/    #active_downlinks = [1,2] #For modules PB/' "$filename"

# Comment out the line containing test_list.extend(test_list_calib)
sed -i '/#active_downlinks = [0/s/.*/    active_downlinks = [0,3]  #For modules PA/' "$filename"

echo "Lines have been modified for a PA module in $filename"

