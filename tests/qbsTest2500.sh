#! /bin/bash

function qbsTargetRun
{
    startsec=${SECONDS}
    ../src/qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} -T ${Target} ${tabu}  >  $tmp_file
    walltime=`echo "print ${SECONDS} - ${startsec}"|python`
    TIME=`grep second $tmp_file| cut -b1-8`
    ENERGY=`grep Energy $tmp_file       | cut -b1-11`
    PARTITIONS=`grep Partitioned $tmp_file       | cut -f1 -d\ `
    echo $test"  "   $TIME "   "$walltime"   " $PARTITIONS"   "  $ENERGY $Local
    Totaltime=`echo "print  $Totaltime + $TIME " |python `
    rm $tmp_file      
}
function testDeltaEnergy
{
echo 
echo " Test non target mode ${testnumber} $numrepeats "
echo "    Test name    CPU sec W/C  Parts   Energy   Delta Energy "   " qbsolv version"
Totaltime=0.0
TotaltimeD=0.0
TotalDenergy=0.0
TotalDenergyD=0.0
SECONDS=0
    for ((i=1;i<11;i++))
    do
       test=${testnumber}_${i}.qubo
       if [ -e ${test_dir}/${test} ] 
       then
           startsec=${SECONDS}
          ../src/qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} $numrepeats > $tmp_file      
           walltime=`echo "print ${SECONDS} - ${startsec}"|python`
            TIME=`grep second $tmp_file       | cut -b1-8`
            PARTITIONS=`grep Partitioned $tmp_file       | cut -f1 -d\ `
              ENERGY=`grep Energy $tmp_file       | cut -b1-11`
              Denergy=`echo "print  ${ENERGY} - ${Energies[i]}"|python `
                echo ${test}"  "   $TIME   "   "$walltime"   " $PARTITIONS"   " $ENERGY "   "$Denergy $Local 
              Totaltime=`echo  "print $Totaltime + $TIME" |python ` 
              TotalDenergy=`echo  "print $TotalDenergy + $Denergy" |python` 
           startsec=${SECONDS}
       fi 
    done
    rm $tmp_file      
  echo "    "Total energy difference $TotalDenergy $Local
  echo "    "Total cpu time $Totaltime $Local
  echo "    "Elapsed time $SECONDS seconds
}


function testDriverTarget
{
Totaltime=0.0
TotaltimeD=0.0
SECONDS=0
echo 
echo "    Test ${testnumber} with Target set "
echo " Test name    CPU sec W/C   Parts     Energy  qbsolv version"
for ((i=1;i<11;i++))
do
    test=${testnumber}_${i}.qubo
    if [ -e $test_dir/$test ] 
    then
       Target=${Energies[i]}
       qbsTargetRun
    fi
done
echo "    "Total cpu time $Totaltime $Local
echo "    "Elapsed time $SECONDS seconds
}


test_dir="qubos"
SubMatrix=""
verbose="-v0"
numrepeats=""
tmp_file=$(mktemp)
tabu="-t 33"
tabu=""

OPTIND=1         # Reset in case getopts has been used previously in the shell.

# Initialize our own variables:

OPTIND=1  #Reset 
while getopts "S:" opt; do
    case "$opt" in
    h)
        echo one option so far -S for submatrix default is no -S
        exit 0
        ;;
    S)  SubMatrix="-S"${OPTARG}
        ;;
    esac
done
shift $((OPTIND-1))

[ "$1" = "--" ] && shift

#Default=`qbsolv -V |grep Version`
Local=`../src/qbsolv -V |grep Compiled|sed "s/Compiled: //"`
echo $SubMatrix  $verbose $numrepeats
echo $Default $Local
dw get env
date
#dw get connection
#dw get solver

testnumber="bqp2500"
Energies=( 99 1515944 1471392 1414192 1507701 1491816 1469162 1479040 1484199 1482413 1483355 )
testDriverTarget
testDeltaEnergy
date
