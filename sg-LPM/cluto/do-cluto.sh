#!/bin/sh

crf="g1p";
numClusters="20";

input="$1";
if [ -z "$input" ]; then
	echo "Missing input file, exiting...";
	exit 1;
fi

scluster -clmethod="direct" -crfun="$crf" "$input" "$numClusters"

coutput="$input.clustering.$numClusters"
foutput="$input.clustering"

mv "$coutput" "$foutput"
