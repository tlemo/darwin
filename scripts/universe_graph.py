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

# generates a dot (https://www.graphviz.org) graph capturing the structure
# of the specified Darwin universe database

import sqlite3
import json
import sys
from collections import defaultdict

if len(sys.argv) != 2:
    print('use universe_graph <universe_db>')
    sys.exit(1)

universe_db = sys.argv[1]

#------------------------------------------------------------------------------
# read the universe objects
#------------------------------------------------------------------------------

db = sqlite3.connect(universe_db)
db.row_factory = sqlite3.Row

cursor = db.cursor()

# read experiments
experiments = {}
for e in cursor.execute('select * from experiment'):
    experiments[e['id']] = e

# read variations
variations = {}
for v in cursor.execute('select * from variation'):
    variations[v['id']] = v

# read evolution traces
traces = {}
for t in cursor.execute('select * from trace'):
    traces[t['id']] = t

#------------------------------------------------------------------------------
# create lookup indexes
#------------------------------------------------------------------------------

variation_traces = defaultdict(list)
for id, trace in traces.items():
    variation_traces[trace['variation_id']].append(id)

#------------------------------------------------------------------------------
# generate the universe graph
#------------------------------------------------------------------------------

arc_normal_style = ''
arc_fork_style = '[style=dashed, color=gray]'

graph = '''
digraph universe {
  rankdir=LR;
  node [shape=circle, color=gray];
  edge [color=black];
'''

# experiments
graph += '  # Experiments\n'
graph += '  { rank=same; node [color=blue, shape=rect];\n'
for id, experiment in experiments.items():
    setup = json.loads(experiment['setup'])
    domain = setup['domain_name']
    population = setup['population_name']
    name = experiment['name'] or ''
    graph += f'    E{id} [label="Exp #{id} {name}|D:{domain}|P:{population}", shape=record]\n'
graph += '  }\n'

# experiment variations
graph += '  # Experiment variations\n'
graph += '  { node [color=black, shape=circle];\n'
for id, variation in variations.items():
    traces_label = '{' + '|'.join(map(str, variation_traces[id])) + '}'
    graph += f'    V{id} [label="v{id}|{traces_label}", shape=Mrecord, fontsize=10]\n'
graph += '  }\n'

# experiment -> last_variation
graph += '  # experiment -> last variation\n'
for id, experiment in experiments.items():
    last_variation_id = experiment['last_variation_id']
    if last_variation_id is not None:
        variation = variations[last_variation_id]
        fork = variation['experiment_id'] != id
        attr = arc_fork_style if fork else arc_normal_style
        graph += f'  E{id} -> V{last_variation_id} {attr}\n'

# variation -> prev_variation
graph += '  # variation -> prev_variation\n'
for id, variation in variations.items():
    previous_id = variation['previous_id']
    if previous_id is not None:
        prev_variation = variations[previous_id]
        fork = prev_variation['experiment_id'] != variation['experiment_id']
        attr = arc_fork_style if fork else arc_normal_style
        graph += f'  V{id} -> V{previous_id} {attr}\n'

graph += '}\n'

print(graph)
