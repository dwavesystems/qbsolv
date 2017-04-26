#! /bin/bash
../src/qbsolv -i qubos/factoring.qubo -m -T 21 $1
../src/qbsolv -i qubos/factoring.qubo -m -T 21 -r 98796  $1
../src/qbsolv -i qubos/factoring.qubo -m -T 21 -r 12345678 $1 
