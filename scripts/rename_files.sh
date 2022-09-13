#!/bin/bash

filenames=`ls ../Images/Research/Synthetic/Results/Cosine/*/*.ppm`
replace_str="consine"
to_replace="euclidean"

for file in $filenames
do
    mv $file "${file/$to_replace/$replace_str}"
done