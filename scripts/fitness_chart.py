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

# creates a fitness chart for the specified evolution trace

import sqlite3
import json
import argparse
import matplotlib.pyplot as plt

#------------------------------------------------------------------------------
# command line parsing
#------------------------------------------------------------------------------

arg_parser = argparse.ArgumentParser(
    description = 'Evolution fitness visualization', allow_abbrev = False)

arg_parser.add_argument('-u', '--universe',
    required = True, help = 'Darwin universe database')
arg_parser.add_argument('-t', '--traceid',
    required = True, help = 'Evolution Trace ID')
arg_parser.add_argument('-o', '--output',
    help = 'Save chart to the specified file (in a format supported by matplotlib)')

args = arg_parser.parse_args()

universe_db = args.universe
trace_id = int(args.traceid)

#------------------------------------------------------------------------------
# read the universe objects
#------------------------------------------------------------------------------

db = sqlite3.connect(universe_db)
db.row_factory = sqlite3.Row

cursor = db.cursor()

# read the fitness values (best/med/worst)
best = []
median = []
worst = []

cursor.execute(
    '''select summary
        from generation
        where trace_id = ?
        order by generation''',
    (trace_id, ))

for generation in cursor:
    summary = json.loads(generation['summary'])
    best.append(summary['best_fitness'])
    median.append(summary['median_fitness'])
    worst.append(summary['worst_fitness'])

#------------------------------------------------------------------------------
# calculate a simple moving average (best fitness values)
# TODO: center the sliding window around the current value?
#------------------------------------------------------------------------------

kSlidingWindowSize = int(len(best) / 20)
moving_average = []
sum = 0
for i in range(len(best)):
    sum += best[i]
    if i >= kSlidingWindowSize:
        sum -= best[i - kSlidingWindowSize]
    count = min(kSlidingWindowSize, i + 1)
    moving_average.append(sum / count)

#------------------------------------------------------------------------------
# plot the fitness values
#------------------------------------------------------------------------------

fig, ax = plt.subplots(figsize=(15, 5))

kAlpha = 0.7
kAvgAlpha = 0.5

plt.plot(best, color = [0, 0, 1, kAlpha], linewidth = 0.5, label = "Best")
plt.plot(median, color = [0.5, 0.5, 0.5, kAlpha], linewidth = 0.5, label = "Median")
plt.plot(worst, color = [1, 0, 0, kAlpha], linewidth = 0.5, label = "Worst")
plt.plot(moving_average, color = [0, 0, 1, kAvgAlpha], linewidth = 3)

plt.grid(True)
plt.tight_layout()
ax.legend(loc='upper left')

if args.output != None:
    print(f'Saving chart to: {args.output}')
    plt.savefig(args.output)

plt.show()
