# Copyright 2018 The Darwin Neuroevolution Framework Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# exports a dot graph representing a NEAT genotype

import sqlite3
import json
import sys
import argparse

#------------------------------------------------------------------------------
# command line parsing
#------------------------------------------------------------------------------

arg_parser = argparse.ArgumentParser(
    description = 'NEAT Genotype Exporter', allow_abbrev = False)

arg_parser.add_argument('-u', '--universe',
    required = True, help = 'Darwin universe database')
arg_parser.add_argument('-t', '--traceid',
    required = True, help = 'Evolution Trace ID')
arg_parser.add_argument('-g', '--generation',
    required = True, help = 'Generation (from which to export the champion genotype)')
arg_parser.add_argument('--explicit-bias',
    help = 'Show the bias node', action='store_true')

args = arg_parser.parse_args()

universe_db = args.universe
trace_id = int(args.traceid)
generation = int(args.generation)

#------------------------------------------------------------------------------
# read the universe objects
#------------------------------------------------------------------------------

db = sqlite3.connect(universe_db)
db.row_factory = sqlite3.Row

cursor = db.cursor()

# read the champion genotype from the specified generation
cursor.execute(
    """select genotypes
        from generation where trace_id = ? and generation = ?""",
        (trace_id, generation))
json_genotypes = json.loads(cursor.fetchone()['genotypes'])
genotype = json_genotypes['champion']

#------------------------------------------------------------------------------
# export the genotype as a graphviz dot specification
#------------------------------------------------------------------------------

kBiasNodeId = 0
kInputs = genotype['inputs']
kOutputs = genotype['outputs']
kInputFirst = 1
kOutputFirst = kInputFirst + kInputs
kHiddenFirst = kOutputFirst + kOutputs
kNodesCount = genotype['nodes_count']

graph = '''
# traceid = %d
# generation = %d
# inputs = %d
# outputs = %d
digraph genotype {
  node [shape=circle, color=gray];
  edge [color=gray];
''' % (trace_id, generation, kInputs, kOutputs)

# bias node
if args.explicit_bias:
    graph += '  # Bias node\n'
    graph += '  { node [label="B", style=dashed, color=black]; 0 }\n'

# input nodes
graph += '  # Input nodes\n'
graph += '  { rank=same; node [color=green]; edge [style=invis]; '
for node_id in range(kInputFirst, kOutputFirst):
    graph += ('%s%d' % (('' if node_id == kInputFirst else '->'), node_id))
graph += ' }\n'

# hidden nodes
graph += '  # Hidden nodes\n'
graph += '  { rank=same; node [color=gray]; edge [style=invis]; '
for node_id in range(kHiddenFirst, kNodesCount):
    graph += ('%s%d' % (('' if node_id == kHiddenFirst else '->'), node_id))
graph += ' }\n'

# output nodes
graph += '  # Output nodes\n'
graph += '  { rank=same; node [color=blue];  edge [style=invis]; '
for node_id in range(kOutputFirst, kHiddenFirst):
    graph += ('%s%d' % (('' if node_id == kOutputFirst else '->'), node_id))
graph += ' }\n'

# links (genes)
graph += '  # Links (genes)\n'
for gene in genotype['genes']:
    in_node = gene['in']
    out_node = gene['out']
    innovation = gene['innovation']
    assert out_node != kBiasNodeId

    if in_node == kBiasNodeId and not args.explicit_bias:
        continue

    if not gene['enabled']:
        arc_style = '[style=dashed, color=gray]'
    elif gene['recurrent']:
        arc_style = '[style=solid, color=brown]'
    else:
        arc_style = '[style=solid, color=gray]'

    graph += f'  {in_node}->{out_node} [label={innovation}]{arc_style};\n'

graph += '}\n'

print(graph)
