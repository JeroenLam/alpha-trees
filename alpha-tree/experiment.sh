#!/bin/bash

lambda=100
increment=1
limit=100
if [ $# -gt 0 ]
then
    lambda=$1
fi

image=11
measure=Cosine
omega=200000
im_path="../Images/Research/Real/source"
out_path="../Images/Research/Real/Results/$measure/$image"

make

while [ $lambda -le $limit ]
do
    for entry in "$im_path"/*$image*.ppm
    do
        file=$(basename $entry .ppm)
        # if [[ $file == *"b"* ]]
        # then
        #     continue
        # fi
        ./saliencetree $entry $lambda $omega $out_path/$measure-$lambda-$file.ppm
    done
    lambda=$(($lambda+$increment))
done
