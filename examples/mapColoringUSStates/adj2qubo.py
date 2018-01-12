"""
Copyright 2016 D-Wave Systems,Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
import csv
import argparse
import os


def ad2_qubo(qubo_out, adj_in):

    # First let's load the adjacency from the file. We store the adjacency
    # information in a dictionary where the keys are the states and the
    # values are sets such that if state0 and state1 are adjacent,
    # (state0 in state_adj[state1]) == True
    # We also want to map each state to an integer index, we store this mapping
    # in state_to_index
    state_adj = {}
    state_to_index = {}
    index_to_state = {}
    idx = 0
    with open(adj_in, "r") as adj:
        for line in adj:
            if line[0] == 'c' or line[0] == '#':
                # skip the comment lines
                continue

            # in each line, there is a comma separated list of states. The
            # first state is the node, the remaining are the nodes adjacent
            # the list is bi-directional (e.g. AK,HI and HI,AK are both lines)
            states = line.strip('\n').split(',')
            state_adj[states[0]] = set(states[1:])

            # each state has an integer label, so we record the mapping in both
            # directions
            state_to_index[states[0]] = idx
            index_to_state[idx] = states[0]
            idx += 1

    # we need some information in advance for the qubo, specifically the number of nodes and
    # the number of couplers
    num_nodes = 4 * len(state_adj)
    num_couplers = sum(len(states) for states in state_adj.values()) * 2 + 6 * len(state_adj)

    # now time to write the qubo
    # nodes first
    with open(qubo_out, "w") as qubo:
        qubo.write("c" + "\n")
        qubo.write("c  this qubo was created by adj2qubo.py for 4 color uninary encoding" + "\n")
        qubo.write("c" + "\n")
        qubo.write("p  qubo  0  {} {} {}\n".format(num_nodes, num_nodes, num_couplers))

        for st in range(len(state_adj)):
            state = index_to_state[st]
            qubo.write("c " + state + "\n")
            qubo.write("  " + str(st * 4) + " " + str(st * 4) + " -1 " + "\n")
            qubo.write("  " + str(st * 4 + 1) + " " + str(st * 4 + 1) + " -1 " + "\n")
            qubo.write("  " + str(st * 4 + 2) + " " + str(st * 4 + 2) + " -1 " + "\n")
            qubo.write("  " + str(st * 4 + 3) + " " + str(st * 4 + 3) + " -1 " + "\n")

        qubo.write("c" + "\n")
        qubo.write("c  Couplers " + "\n")
        qubo.write("c" + "\n")

        for st in range(len(state_adj)):
            state = index_to_state[st]
            neighbors = state_adj[state]
            qubo.write("c " + state + "   " + str(len(neighbors)) + " neighbors  " +
                       str(len(neighbors) * 4) + " external couplers\n")
            qubo.write("  " + str(st * 4) + " " + str(st * 4 + 1) + " 2 " + "\n")
            qubo.write("  " + str(st * 4) + " " + str(st * 4 + 2) + " 2 " + "\n")
            qubo.write("  " + str(st * 4) + " " + str(st * 4 + 3) + " 2 " + "\n")
            qubo.write("  " + str(st * 4 + 1) + " " + str(st * 4 + 2) + " 2 " + "\n")
            qubo.write("  " + str(st * 4 + 1) + " " + str(st * 4 + 3) + " 2 " + "\n")
            qubo.write("  " + str(st * 4 + 2) + " " + str(st * 4 + 3) + " 2 " + "\n")

            for ext_coup in sorted(state_to_index[state] for state in neighbors):
                if ext_coup < st:
                    continue
                coupl_st = index_to_state[ext_coup]
                qubo.write("c " + state + " linked to   " + coupl_st + "\n")

                qubo.write("  " + str(st * 4 + 0) + " " + str(ext_coup * 4 + 0) + " 1 " + "\n")
                qubo.write("  " + str(st * 4 + 1) + " " + str(ext_coup * 4 + 1) + " 1 " + "\n")
                qubo.write("  " + str(st * 4 + 2) + " " + str(ext_coup * 4 + 2) + " 1 " + "\n")
                qubo.write("  " + str(st * 4 + 3) + " " + str(ext_coup * 4 + 3) + " 1 " + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Read adj files and create a qubo 4 color map file')
    parser.add_argument("-i", "--adj", help="Input adjacency graph file ", required=True)
    parser.add_argument("-o", "--qubo", type=str, help="output .qubo file", required="True")
    parser.add_argument("-v", "--verbosity", action="store_true", help="Verbosity level", default=0)

    args = parser.parse_args()

    adj_in = args.adj
    qubo_out = args.qubo
    ad2_qubo(qubo_out, adj_in)
