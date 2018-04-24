#!/bin/bash
# $1 - tree string

# EXAMPLE:
#   ./test.sh ABCDEFG

# where
#            A
#          /    \
#        B        C
#      /    \    /   \
#     D     E   F     G

LC_ALL=C

tree=$1
len=${#tree}
if [[ "$len" == "1" ]]; then
    echo "$tree"
    exit 0
fi
len=$((len * 2 - 2))

# compile
mpic++ --prefix /usr/local/share/OpenMPI -o pro pro.cpp

# run
mpirun --prefix /usr/local/share/OpenMPI -np $len pro $tree

rm -f pro
