#!/bin/bash

# amt_args=$#

# if [ $amt_args -lt 2 ]
# then 
#     echo "Use script as"
#     echo "./convert_to_png.sh <directory_of_ppm_images> <output_directory>"
#     exit
# fi

search_dir="../Images/Research/Real/source"
out_dir="../Images/Research/Real/source/png"

for entry in "$search_dir"/*b*.ppm
do
    file=$(basename $entry .ppm)
    convert $entry $out_dir/$file.png
done