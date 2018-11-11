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

# generates a human readable summary of the universe

import sqlite3
import json
import argparse
import math
import time
import textwrap
from collections import defaultdict

#------------------------------------------------------------------------------
# command line parsing
#------------------------------------------------------------------------------

arg_parser = argparse.ArgumentParser(
    description = 'Darwin Universe Summary', allow_abbrev = False)

arg_parser.add_argument('universe',
    help = 'Darwin universe database')
arg_parser.add_argument('-s', '--stats',
    help = 'Gather evolution trace state (can be very slow if there are many generations)',
    action='store_true')
arg_parser.add_argument('-v', '--verbose',
    help = 'Show detailed information', action='store_true')

args = arg_parser.parse_args()

universe_db = args.universe

#------------------------------------------------------------------------------
# read the universe objects
#------------------------------------------------------------------------------

class TraceSummary:
    def __init__(self):
        self.generations_count = 0
        self.max_fitness = -math.inf

db = sqlite3.connect(universe_db)
db.row_factory = sqlite3.Row

cursor = db.cursor()

print('\nLoading universe objects...');

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

trace_summary = defaultdict(TraceSummary)

# scan all the generations, tracking the following values:
#   - count of generations per trace
#   - max fitness
#
# NOTE: since this can be very slow when the universe contains
#   many generations, it is optional
if args.stats:
    for g in cursor.execute('select * from generation'):
        results_summary = json.loads(g['summary'])
        trace_id = g['trace_id']
        summary = trace_summary[trace_id]
        summary.generations_count += 1
        summary.max_fitness = max(summary.max_fitness, results_summary['best_fitness'])

#------------------------------------------------------------------------------
# create reverse lookup indexes
#------------------------------------------------------------------------------

experiment_variations = defaultdict(list)
for id, variation in variations.items():
    experiment_variations[variation['experiment_id']].append(id)

variation_traces = defaultdict(list)
for id, trace in traces.items():
    variation_traces[trace['variation_id']].append(id)

#------------------------------------------------------------------------------
# output the universe summary
#------------------------------------------------------------------------------

for e_id, experiment in experiments.items():
    setup = json.loads(experiment['setup'])
    domain = setup['domain_name']
    population = setup['population_name']
    population_size = setup['population_size']
    name = experiment['name'] or ''
    print(f'\nExperiment #{e_id} : {domain} / {population}[{population_size}] {name}')
    for v_id in experiment_variations[e_id]:
        print(f'  Variation #{v_id} :')
        for t_id in variation_traces[v_id]:
            summary = trace_summary[t_id]
            trace = traces[t_id]
            date_time = time.strftime(
                '%Y-%m-%d, %H:%M:%S',
                time.localtime(trace['timestamp']))
            trace_str = f'    Trace #{t_id} : {date_time}'
            if args.stats:
                trace_str += f' ({summary.generations_count} generations'
                trace_str += f', max_fitness = {summary.max_fitness:.2f})'
            print(trace_str)
            if args.verbose:
                print(textwrap.indent(trace['evolution_config'], '    '))
