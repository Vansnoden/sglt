#!/bin/sh

cdir="compd";

if ! [ -z "$1" ]; then
	cdir="$1";
fi


out="build";
mkdir -p "$out"
rm -rf "$out/"*;

cp -r "$cdir/"* "$out/."

LPMPATH=`pwd`"/../lpm"
LPMPATHOUT="$LPMPATH/build"


sed -i "s&%LPM_ROOTPATH%&$LPMPATH&" "$out/subdir.mk"
sed -i "s&%LPM_LIBPATH%&$LPMPATHOUT&" "$out/makefile"


cd "$out" && make clean && make all;
