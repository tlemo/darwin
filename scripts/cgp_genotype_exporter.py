# Copyright 2019 The Darwin Neuroevolution Framework Authors.
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
    description = 'CGP Genotype Exporter', allow_abbrev = False)

arg_parser.add_argument('-u', '--universe',
    required = True, help = 'Darwin universe database')
arg_parser.add_argument('-t', '--traceid',
    required = True, help = 'Evolution Trace ID')
arg_parser.add_argument('-g', '--generation',
    required = True, help = 'Generation (from which to export the champion genotype)')

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
# genotype values and helpers
#------------------------------------------------------------------------------

constants_genes = genotype['constants_genes']
function_genes = genotype['function_genes']
output_genes = genotype['output_genes']

kInputs = genotype['inputs']
kOutputs = genotype['outputs']
kRows = genotype['rows']
kColumns = genotype['columns']
kConstants = len(constants_genes)

def inputNode(i):
    assert i >= 0 and i < kInputs
    return 'i%d' % i

def functionNode(col, row):
    assert col >= 0 and col < kColumns
    assert row >= 0 and row < kRows
    return 'f%d' % (kInputs + col * kRows + row)

def outputNode(i):
    assert i >= 0 and i < kOutputs
    return 'o%d' % (kInputs + kColumns * kRows + i)

def indexToNode(i):
    if i < kInputs:
        return inputNode(i)
    elif i < kInputs + kColumns * kRows:
        fn = i - kInputs
        col = fn // kRows
        row = fn % kRows
        return functionNode(col, row)
    else:
        return outputNode(i - kInputs - kColumns * kRows)

#------------------------------------------------------------------------------
# identify the active nodes
#------------------------------------------------------------------------------

active_nodes = set()

def nodeDfs(index):
    node = indexToNode(index)
    if node not in active_nodes:
        active_nodes.add(node)
        if index >= kInputs:
            gene = function_genes[index - kInputs]
            arity = gene['a']
            for src_index in gene['c'][0:arity]:
                nodeDfs(src_index)

for gene in output_genes:
    nodeDfs(gene['c'])

#------------------------------------------------------------------------------
# export the genotype as a graphviz dot specification
#------------------------------------------------------------------------------

graph = '''
# traceid = %d
# generation = %d
digraph genotype {
  rankdir=LR
  compound=true
  splines=true
  graph [fontsize=6]
  node [shape=circle; fontsize=8; margin=0; color="#80808080"; fontcolor="#80808080"]
  edge [color="#80808080"]
''' % (trace_id, generation)

# input nodes
graph += '\n  # Input nodes\n'
graph += '  subgraph cluster_inputs {\n'
graph += '    label="inputs"\n'
graph += '    color=lightgrey\n'
for i in range(kInputs):
    node = inputNode(i)
    if node in active_nodes:
        graph += '    %s [color=blue; fontcolor=blue]\n' % node
graph += '    { rank=same; edge [style=invis;]; '
for i in reversed(range(kInputs)):
    node = inputNode(i)
    graph += f"{node}%s" % ('' if i == 0 else '->');
graph += ' }\n'
graph += '  }\n'

# function nodes
graph += '\n  # Function nodes\n'
graph += '  subgraph cluster_functions {\n'
graph += '    color=lightgrey\n'
for col in range(kColumns):
    graph += '    subgraph cluster_col%d {' % col
    graph += ' label="col %d";' % col
    graph += ' color=lightgrey;\n'
    graph += '      { rank=same; edge [style=invis]; '
    for row in reversed(range(kRows)):
        node = functionNode(col, row)
        graph += f'{node}%s' % ('' if row == 0 else '->')
    graph += ' }}\n'
graph += '  }\n'

# output nodes
graph += '\n  # Output nodes\n'
graph += '  subgraph cluster_outputs {\n'
graph += '    label="outputs"\n'
graph += '    color=lightgrey\n'
graph += '    node [color=blue; fontcolor=blue]\n'
graph += '    { rank=same; edge [style=invis;]; '
for i in reversed(range(kOutputs)):
    node = outputNode(i)
    graph += f"{node}%s" % ('' if i == 0 else '->');
graph += ' }\n'
graph += '  }\n'

# layout everything: inputs -> functions -> outputs
graph += '\n  { edge [style=invis]; %s->' % inputNode(kInputs // 2)
for col in range(kColumns):
    graph += '%s->' % functionNode(col, kRows // 2)
graph += '%s }\n' % outputNode(kOutputs // 2)

# function genes
graph += '\n  # Function genes\n'
for i in range(len(function_genes)):
    gene = function_genes[i]
    node = indexToNode(kInputs + i)
    active = node in active_nodes
    color_attr = ' color=blue; fontcolor=blue' if active else ''
    graph += '  %s [label="%s";%s]\n' % (node, gene['fn'], color_attr)
    fn_arity = gene['a']
    edge_attr = 'constraint=false;%s' % (' color=blue' if active else '')
    for src_index in gene['c'][0:fn_arity]:
        graph += '  %s->%s [%s]\n' % (indexToNode(src_index), node, edge_attr)

# output genes
graph += '\n  # Output genes\n'
for i in range(len(output_genes)):
    gene = output_genes[i]
    node = outputNode(i)
    src = indexToNode(gene['c'])
    graph += '  %s->%s [constraint=false; color=blue]\n' % (src, node)

# done
graph += '}\n'

print(graph)
