#! /bin/bash

function qbsTargetRun
{
    startsec=${SECONDS}
    ../src/qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} -T ${Target} ${option1} >  $tmp_file
    walltime=`echo "print ${SECONDS} - ${startsec}"|python`
    TIME=`grep second $tmp_file| cut -b1-8`
    ENERGY=`grep Energy $tmp_file       | cut -b1-11`
    PARTITIONS=`grep Partitioned $tmp_file       | cut -f1 -d\ `
    echo $test"  "   $TIME "   "$walltime"   " $PARTITIONS"   "  $ENERGY $Local  $option1
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
TotalenergyBest=0.0
SECONDS=0
    for ((i=1;i<11;i++))
    do
       test=${testnumber}_${i}.qubo
       if [ -e ${test_dir}/${test} ] 
       then
           startsec=${SECONDS}
          ../src/qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} ${numrepeats} ${option1}> $tmp_file      
           walltime=`echo "print ${SECONDS} - ${startsec}"|python`
            TIME=`grep second $tmp_file       | cut -b1-8`
            PARTITIONS=`grep Partitioned $tmp_file       | cut -f1 -d\ `
              ENERGY=`grep Energy $tmp_file       | cut -b1-11`
              Denergy=`echo "print  ${ENERGY} - ${Energies[i]}"|python `
              TotalenergyBest=`echo "print  ${TotalenergyBest} + ${Energies[i]}"|python `
                echo ${test}"  "   $TIME   "   "$walltime"   " $PARTITIONS"   " $ENERGY "   "$Denergy $Local $option1
              Totaltime=`echo  "print $Totaltime + $TIME" |python ` 
              TotalDenergy=`echo  "print $TotalDenergy + $Denergy" |python` 
           startsec=${SECONDS}
       fi 
    done
    rm $tmp_file      
  echo "    "Total energy difference $TotalDenergy $Local
  TotalenergyPercent=`echo "print  $TotalDenergy / ${TotalenergyBest} * 100. "|python `
  echo "    "Total energy difference Percent $TotalenergyPercent $Local
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

option1=$1
test_dir="qubos"
SubMatrix=""
verbose="-v0"
numrepeats=""
tmp_file=$(mktemp)

OPTIND=1         # Reset in case getopts has been used previously in the shell.

# Initialize our own variables:

OPTIND=1  #Reset 

#Default=`qbsolv -V |grep Version`
Local=`../src/qbsolv -V |grep Compiled|sed "s/Compiled: //"`
echo $SubMatrix  $verbose $numrepeats
echo $Default $Local
#dw get env
#date
#dw get connection
#dw get solver

testnumber="bqp2500"
Energies=( 99 1515944 1471392 1414192 1507701 1491816 1469162 1479040 1484199 1482413 1483355 )
testDriverTarget
testDeltaEnergy
date
