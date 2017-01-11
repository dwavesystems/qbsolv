#!/bin/bash
#  Copyright 2016 D-Wave Systems,Inc.
#   
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  					 
#  http://www.apache.org/licenses/LICENSE-2.0
#  									 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

adj=${1:-"blank_US_state_map.svg"}
prwd=$(pwd)

for qbouts in *.qbout
do  echo $qbouts $adj $xsplits
    split -l 5 $qbouts
    for xsplits in x*
    do  python color_states.py -i $prwd/$xsplits -s $prwd/${adj} -o $prwd/${qbouts}.${xsplits}.svg
    done
    rm $prwd/x*
done

