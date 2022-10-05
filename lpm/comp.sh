#!/bin/sh

cdir="compd";

if ! [ -z "$1" ]; then
	cdir="$1";
fi


out="build";
mkdir -p "$out"
rm -rf "$out/"*;

cp -r "$cdir/"* "$out/."

cd "$out" && make clean && make all;
