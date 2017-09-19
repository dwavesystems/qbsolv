#!/bin/bash
 
echo "**************** qbsolv: map coloring"
{
    cd mapColoringUSStates
    echo ---- example of a full application using qbsolv
    echo -----
    ./demoStates.sh
    echo ---- open in a browser the .svg files to see a colored map
    echo ----
    cd ..
} >mapColoring.out 2>&1

echo "**************** qbsolv: python interface"
{
    echo --- using the python interface to qbsolv example
    echo ---
    python tryDwaveQbsolv.py
} >tryDwave.out 2>&1

echo "**************** qbsolv: factoring"
./factoring.bash >factoring.out 2>&1
