### adj2qubo.py 
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

def ad2_qubo(qubo_out,adj_in):
 
    import csv
    max_states=1000
    qubo_Records=[0] * max_states
    states_records=[0] * max_states

    states_adj=[0] * max_states
    states_couplers=[0] * max_states
    states=[0]*max_states
    states_num=[0]*max_states
    qubo = open (qubo_out,"w")
    adj = open (adj_in,"r")
    
    adj_lines = []
    num_states = 0
    for line in adj:
        if line[0]=='c' or line[0]=='#':
            continue
        adj_lines.append(line.strip('\n'))
        num_states += 1
    num_nodes=0 
    num_couplers=0
    for i in range ( num_states ):
        states_records[i]=adj_lines[i].split(',')
        states_adj[i]=len(states_records[i]) -1
        states[i]="".join(map(str,states_records[i][:1]))
        states_records[i]=[ it for it in states_records[i] if it != states[i]]
        num_nodes+=1

    for i in range ( num_states ):
        state_str="".join(map(str,states[i]))
        states_num[i]=i
        for k in range ( num_states ):
            states_records[k]=[ it if it != state_str else i for it in states_records[k] ]
            states_records[k]=[ it for it in states_records[k] if it > k ]

        states_couplers[i]=len(states_records[i])
        num_couplers+=states_couplers[i]

    num_couplers=num_couplers*4+num_states*6
    num_nodes=num_nodes*4

# now time to write the qubo
# nodes first
    qubo.write("c"+"\n")
    qubo.write("c  this qubo was created by adj2qubo.py for 4 color uninary encoding"+"\n")
    qubo.write("c"+"\n")
    qubo.write("p  qubo  0  " + str(num_nodes) + " " + str(num_nodes) + " " + str(num_couplers) + "\n" )

    for st in range(num_states):
        qubo.write("c " + states[st] + "\n")
        qubo.write("  "+str(st*4) + " " + str(st*4) + " -1 "+ "\n" )
        qubo.write("  "+str(st*4+1) + " " + str(st*4+1) + " -1 "+ "\n" )
        qubo.write("  "+str(st*4+2) + " " + str(st*4+2) + " -1 "+ "\n" )
        qubo.write("  "+str(st*4+3) + " " + str(st*4+3) + " -1 "+ "\n" )

    qubo.write("c"+"\n")
    qubo.write("c  Couplers "+"\n")
    qubo.write("c"+"\n")

    for st in range(num_states):
        qubo.write("c " + states[st] + "   "+str(states_adj[st])+" neighbors  "+str(states_couplers[st]*4)+" external couplers\n")
        qubo.write("  "+str(st*4)   + " " + str(st*4+1) + " 2 "+ "\n" )
        qubo.write("  "+str(st*4)   + " " + str(st*4+2) + " 2 "+ "\n" )
        qubo.write("  "+str(st*4)   + " " + str(st*4+3) + " 2 "+ "\n" )
        qubo.write("  "+str(st*4+1) + " " + str(st*4+2) + " 2 "+ "\n" )
        qubo.write("  "+str(st*4+1) + " " + str(st*4+3) + " 2 "+ "\n" )
        qubo.write("  "+str(st*4+2) + " " + str(st*4+3) + " 2 "+ "\n" )

        for ext_coup in range(states_couplers[st]) :
            coupl_st=states_records[st][ext_coup]
            qubo.write("c " + states[st] + " linked to   "+states[coupl_st]+ "\n")

            qubo.write("  "+str(st*4+0)   + " " + str(coupl_st*4+0) + " 1 "+ "\n" )
            qubo.write("  "+str(st*4+1)   + " " + str(coupl_st*4+1) + " 1 "+ "\n" )
            qubo.write("  "+str(st*4+2)   + " " + str(coupl_st*4+2) + " 1 "+ "\n" )
            qubo.write("  "+str(st*4+3)   + " " + str(coupl_st*4+3) + " 1 "+ "\n" )
            


    
    adj.close()
    qubo.close()

# end the thing

if __name__ == "__main__":
    import argparse
    import os
    parser = argparse.ArgumentParser(description='Read adj files and create a qubo 4 color map file')
    parser.add_argument("-i","--adj", help="Input adjacency graph file ",required=True)
    parser.add_argument("-o","--qubo" , type=str, help="output .qubo file",required="True")
    parser.add_argument("-v","--verbosity",action="store_true",help="Verbosity level",default=0)

    args = parser.parse_args()

    adj_in=args.adj
    qubo_out=args.qubo
    ad2_qubo(qubo_out,adj_in)

