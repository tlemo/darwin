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

# Creates a fitness chart for the aggregated traces under the specified variation

import sqlite3
import json
import argparse
import matplotlib.pyplot as plt
from collections import defaultdict
from random import random

#------------------------------------------------------------------------------
# command line parsing
#------------------------------------------------------------------------------

arg_parser = argparse.ArgumentParser(
    description = 'Evolution fitness visualization', allow_abbrev = False)

arg_parser.add_argument('-u', '--universe',
    required = True, help = 'Darwin universe database')
arg_parser.add_argument('-v', '--variationid',
    required = True, help = 'Experiment Variation ID')
arg_parser.add_argument('--calibration',
    help = 'Plot the calibration fitness values instead of the best/median/worst values',
    action='store_true')
arg_parser.add_argument('-o', '--output',
    help = 'Save chart to the specified file (in a format supported by matplotlib)')

args = arg_parser.parse_args()

universe_db = args.universe
variation_id = int(args.variationid)

#------------------------------------------------------------------------------
# setup the chart
#------------------------------------------------------------------------------

fig, ax = plt.subplots(figsize=(15, 5))

def seriesColor(series_name):
    return {
        'best'      : [0, 0, 1],
        'median'    : [0.5, 0.5, 0.5],
        'worst'     : [1, 0, 0],
    }.get(series_name, [random(), random(), random()])

#------------------------------------------------------------------------------
# aggregate the traces and plot the average & min..max range
#------------------------------------------------------------------------------

# series[series_name][generation] = [...values...]
series = defaultdict(lambda: defaultdict(list))

def processTrace(trace_id):
    cursor = db.cursor()
    cursor.execute(
        '''select generation, summary
            from generation
            where trace_id = ?''',
        (trace_id, ))

    for generation in cursor:
        generation_index = generation['generation']
        summary = json.loads(generation['summary'])
        if args.calibration:
            for cf_name, cf_value in summary['calibration_fitness'].items():
                series[cf_name][generation_index].append(cf_value)
        else:
            series['best'][generation_index].append(summary['best_fitness'])
            series['median'][generation_index].append(summary['median_fitness'])
            series['worst'][generation_index].append(summary['worst_fitness'])

db = sqlite3.connect(universe_db)
db.row_factory = sqlite3.Row

cursor = db.cursor()
cursor.execute(
    '''select id
        from trace
        where variation_id = ?''',
    (variation_id, ))

for trace in cursor:
    processTrace(trace['id'])

for series_name, series_data in series.items():
    series_color = seriesColor(series_name)
    generations = series_data.keys()
    values = series_data.values()
    plt.fill_between(generations,
        [min(values_set) for values_set in values],
        [max(values_set) for values_set in values],
        color = [*series_color, 0.25])
    plt.plot(generations,
        [sum(values_set) / len(values_set) for values_set in values],
        color = series_color,
        label = series_name)

#------------------------------------------------------------------------------
# finalize the chart
#------------------------------------------------------------------------------

plt.grid(True)
ax.legend(loc='upper left')

if args.output != None:
    print(f'Saving chart to: {args.output}')
    plt.savefig(args.output)

plt.show()
