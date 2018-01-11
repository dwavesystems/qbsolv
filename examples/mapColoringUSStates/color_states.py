# color_map.py
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


def color_map(qsolv_out, svg_out, svg_in):

    import csv

    # Map colors
    colors = ["f0f0f0", "#4daf4a", "#377eb8", "#e41a1c", "#feb24c"]
    #           white      green     blue      red     yellow
    # colors = ["#f0f0f0","#238b45","#2171b5","#cb181d","#ffeda0"]
    #           white      green     blue      red     yellow
    # colors = ["#f0f0f0","#238b45","#2171b5","#cb181d","#8856a7"]
    #           white      green     blue      red     purple
    # colors = ["#f0f0f0","#d7191c","#fdae61","#abd9e9","#2c7bb6"]

    qbsolv = open(qsolv_out, "r")
    lines = qbsolv.read()
    lines = lines.split('\n')
    qbits = lines[0].split()
    nqbits = int(qbits[0])
    count = [0] * 5
    print "number of states ", (nqbits + 1) / 4
    County_colors = [0] * (nqbits / 4)

    for i in range(0, nqbits, 4):
        bits = lines[1]
        ivar = i / 4
        color_class = [ndx for b, ndx in zip(bits[i:i + 4], range(1, 5)) if b == "1"][0]
        count[color_class] += 1
        County_colors[ivar] = color_class

    print "W  G  B  R  Y"
    print ''.join([str(it) + " " for it in count])

    qbsolv.close()

    # Read in ids

    idcolors = {}
    svg = open(svg_in, "r")
    lines = svg.read()
    lines = lines.split('\n')
    svg.close

    firstln = lines[0].split()
    numLinesSkip = 4

    svgHdr = """
<!-- 
This file is adapted from an uncolored-US-states SVG file at 
https://commons.wikimedia.org/wiki/File:Blank_US_Map_(states_only).svg

License: GNU Free Documentation License, version 1.2 and Creative Commons 
    Attribution-Share Alike 3.0 Unported.
- GNU Free Documentation License, version 1.2: Permission is granted to copy, 
distribute and/or modify this document under the terms of the GNU Free 
Documentation License, Version 1.2 or any later version published by
the Free Software Foundation; with no Invariant Sections, no Front-Cover
Texts, and no Back-Cover Texts. A copy of the license is included in the 
section entitled GNU Free Documentation License
(https://en.wikipedia.org/wiki/GNU_Free_Documentation_License).
- Creative Commons Attribution-Share Alike 3.0 Unported: This file is
licensed under the Creative Commons Attribution-Share Alike 3.0 Unported
license (https://creativecommons.org/licenses/by-sa/3.0/legalcode).
-->
		"""
    fout = open(svg_out, "w")
    for record in range(0, numLinesSkip):
        fout.write(lines[record])
        fout.write("\n")

    fout.write(svgHdr)

    for record in range((nqbits / 4)):
        line = lines[numLinesSkip + record].split("=")
        line[2] = '"' + colors[County_colors[record]] + '" d'

        if County_colors[record] == colors[0]:
            print " White ", line[1]
        for i in range(len(line) - 1):
            fout.write(line[i] + "=")
        fout.write(line[-1])
        fout.write("\n")

    fout.write(lines[-4])
    fout.write("\n")
    fout.write(lines[-3])
    fout.write("\n")
    fout.write(lines[-2])
    fout.write("\n")
    fout.write(lines[-1])
    fout.close


if __name__ == "__main__":
    import argparse
    import os
    parser = argparse.ArgumentParser(
        description='Read .qubo.out files combine with Adjacency file and create an svg file')
    parser.add_argument("-s", "--unc", help="Input uncolored svg file", required=True)
    parser.add_argument("-i", "--sol", help="Input solution file from qbsolv ", required=True)
    parser.add_argument("-o", "--svg", type=str, help="output map SVG file", required="True")
    parser.add_argument("-v", "--verbosity", action="store_true", help="Verbosity level", default=0)

    args = parser.parse_args()

    qsolv_out = args.sol
    svg_in = args.unc
    svg_out = args.svg
    color_map(qsolv_out, svg_out, svg_in)
