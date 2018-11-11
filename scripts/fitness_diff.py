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

# creates overlapping fitness charts from multiple traces
#
# TODO:
#   - check that the traces correspond to similar domains
#   - allow more than two traces (arbritrary number of traces?)

import sqlite3
import json
import argparse
import matplotlib.pyplot as plt

#------------------------------------------------------------------------------
# command line parsing
#------------------------------------------------------------------------------

arg_parser = argparse.ArgumentParser(
    description = 'Fitness diff tool', allow_abbrev = False)

arg_parser.add_argument('-u', '--universe',
    required = True, help = 'Darwin universe database')
arg_parser.add_argument('-tb', '--base_traceid',
    required = True, help = 'Evolution Trace ID')
arg_parser.add_argument('-td', '--diff_traceid',
    required = True, help = 'Evolution Trace ID')
arg_parser.add_argument('-o', '--output',
    help = 'Save chart to the specified file (in a format supported by matplotlib)')
arg_parser.add_argument('--all-generations',
    help = "Don't limit the chart to the shared generations only", action='store_true')

args = arg_parser.parse_args()

universe_db = args.universe
base_trace_id = int(args.base_traceid)
diff_trace_id = int(args.diff_traceid)

#------------------------------------------------------------------------------
# read the universe objects
#------------------------------------------------------------------------------

db = sqlite3.connect(universe_db)
db.row_factory = sqlite3.Row

def bestFitnessValues(trace_id):
    cursor = db.cursor()
    cursor.execute(
        '''select summary
            from generation
            where trace_id = ?
            order by generation''',
        (trace_id, ))
    values = []    
    for generation in cursor:
        summary = json.loads(generation['summary'])
        values.append(summary['best_fitness'])
    return values
    
# read the fitness values
base = bestFitnessValues(base_trace_id)
diff = bestFitnessValues(diff_trace_id)

# limit to the shared generations by default
if not args.all_generations:
    common = min(len(base), len(diff))
    base = base[:common]
    diff = diff[:common]

#------------------------------------------------------------------------------
# calculate a simple moving average
#------------------------------------------------------------------------------

def movingAverage(values, sliding_window_size):
    moving_average = []
    sum = 0
    for i in range(len(values)):
        sum += values[i]
        if i >= sliding_window_size:
            sum -= values[i - sliding_window_size]
        count = min(sliding_window_size, i + 1)
        moving_average.append(sum / count)
    return moving_average

kSlidingWindowSize = max(int(len(base) / 20), 1)
base_moving_average = movingAverage(base, kSlidingWindowSize)
diff_moving_average = movingAverage(diff, kSlidingWindowSize)

#------------------------------------------------------------------------------
# plot the fitness values
#------------------------------------------------------------------------------

fig, ax = plt.subplots(figsize=(15, 5))

kAlpha = 0.3
kAvgAlpha = 0.7

plt.plot(base, color = [0.3, 0.3, 0.3, kAlpha], linewidth = 0.5)
plt.plot(
    base_moving_average,
    color = [0.3, 0.3, 0.3, kAvgAlpha],
    linewidth = 2,
    label = "Base")

plt.plot(diff, color = [0, 0, 1, kAlpha], linewidth = 0.5)
plt.plot(
    diff_moving_average,
    color = [0, 0, 1, kAvgAlpha],
    linewidth = 2,
    label = "Diff")

plt.grid(True)
ax.legend(loc='upper left')

if args.output != None:
    print(f'Saving chart to: {args.output}')
    plt.savefig(args.output)

plt.show()
