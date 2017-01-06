#! /bin/bash

function qbsTargetRun
{
    startsec=${SECONDS}
    ../src/qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} -T ${Target}  >  $tmp_file
    walltime=`echo "print ${SECONDS} - ${startsec}"|python`
    TIME=`grep second $tmp_file| cut -b1-8`
    ENERGY=`grep Energy $tmp_file       | cut -b1-11`
    PARTITIONS=`grep Partitioned $tmp_file       | cut -f1 -d\ `
    echo $test"  "   $TIME "   "$walltime"   " $PARTITIONS"   "  $ENERGY $Local
    Totaltime=`echo "print  $Totaltime + $TIME " |python `
    startsec=${SECONDS}
#    echo "qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} -T ${Target} "
    qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} -T ${Target} > $tmp_file       
    walltime=`echo "print ${SECONDS} - ${startsec}"|python`
    TIME=`grep second $tmp_file       | cut -b1-8`
    ENERGY=`grep Energy $tmp_file       | cut -b1-11`
    PARTITIONS=`grep Partitioned $tmp_file       | cut -f1 -d\ `
    TotaltimeD=`echo "print  $TotaltimeD + $TIME " |python `
    echo $test"  "   $TIME "   "$walltime"   " $PARTITIONS"   "  $ENERGY  $Default
    #Totaltime=`echo "print  $Totaltime + $TIME " |python `
    #echo $Totaltime
    #rm $tmp_file      
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
          ../src/qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} > $tmp_file      
           walltime=`echo "print ${SECONDS} - ${startsec}"|python`
            TIME=`grep second $tmp_file       | cut -b1-8`
            PARTITIONS=`grep Partitioned $tmp_file       | cut -f1 -d\ `
              ENERGY=`grep Energy $tmp_file       | cut -b1-11`
              Denergy=`echo "print  ${ENERGY} - ${Energies[i]}"|python `
                echo ${test}"  "   $TIME   "   "$walltime"   " $PARTITIONS"   " $ENERGY "   "$Denergy $Local 
              Totaltime=`echo  "print $Totaltime + $TIME" |python ` 
              TotalDenergy=`echo  "print $TotalDenergy + $Denergy" |python` 
           startsec=${SECONDS}
          qbsolv -i ${test_dir}/${test} -m ${SubMatrix} ${verbose} $numrepeats > $tmp_file      
           walltime=`echo "print ${SECONDS} - ${startsec}"|python`
            TIME=`grep second $tmp_file       | cut -b1-8`
            PARTITIONS=`grep Partitioned $tmp_file       | cut -f1 -d\ `
              ENERGY=`grep Energy $tmp_file       | cut -b1-11`
              Denergy=`echo "print  ${ENERGY} - ${Energies[i]}"|python `
              TotaltimeD=`echo  "print $TotaltimeD + $TIME" |python ` 
              TotalDenergyD=`echo  "print $TotalDenergyD + $Denergy" |python` 
                echo ${test}"  "   $TIME  "   "$walltime"   " $PARTITIONS"   " $ENERGY "   "$Denergy $Default
       fi 
    done
    rm $tmp_file      
  echo "    "Total energy difference $TotalDenergy $Local
  echo "    "Total energy difference $TotalDenergyD $Default
  echo "    "Total cpu time $Totaltime $Local
  echo "    "Total cpu time $TotaltimeD $Default
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
echo "    "Total cpu time $TotaltimeD $Default
echo "    "Elapsed time $SECONDS seconds
}


test_dir="qubos"
SubMatrix=""
verbose="-v0"
numrepeats=""
tmp_file=$(mktemp)

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

Default=`qbsolv -V |grep Version`
Local=`../src/qbsolv -V |grep Compiled|sed "s/Compiled: //"`
echo $SubMatrix  $verbose $numrepeats
echo $Default $Local
#dw get env
date
#dw get connection
#dw get solver


testnumber="bqp50"
Energies=( 99 2098 3702 4626 3544 4012 3693 4520 4216 3780 3507 )
testDriverTarget
testDeltaEnergy

testnumber="bqp100"
Energies=( 99 7970 11036 12723 10368 9083 10210 10125 11435 11455 12565 )
testDriverTarget
testDeltaEnergy

testnumber="bqp250"
Energies=( 99 45607 44810 49037 41274 47961 41014 46757 35726 48916 40442 )
testDriverTarget
testDeltaEnergy

testnumber="bqp500"
Energies=( 99 116586 128339 130812 130097 125487 121772 122201 123559 120798 130619 )
testDriverTarget
testDeltaEnergy

testnumber="bqp1000"
Energies=( 99 371438 354932 371236 370675 352760 359629 371193 351994 349337 351415 )
testDriverTarget
testDeltaEnergy

testnumber="bqp2500"
Energies=( 99 1515944 1471392 1414192 1507701 1491816 1469162 1479040 1484199 1482413 1483355 )
testDriverTarget
testDeltaEnergy
date
