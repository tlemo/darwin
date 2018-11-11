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

# plots the fitness distribution over multiple generations
# (using an 'onion skinning' technique to show the evolution of distributions)

import sqlite3
import json
import argparse
import sys
import seaborn as sns
import matplotlib.pyplot as plt

#------------------------------------------------------------------------------
# command line parsing
#------------------------------------------------------------------------------

arg_parser = argparse.ArgumentParser(
    description = 'Fitness distribution visualization', allow_abbrev = False)

arg_parser.add_argument('-u', '--universe',
    required = True, help = 'Darwin universe database')
arg_parser.add_argument('-t', '--traceid',
    required = True, help = 'Evolution Trace ID')
arg_parser.add_argument('-s', '--sample-percent', default=10,
    help = 'The percent of generations to plot')
arg_parser.add_argument('--sorted',
    help = "Plot the raw, sorted fitness values", action='store_true')
arg_parser.add_argument('-o', '--output',
    help = 'Save chart to the specified file (in a format supported by matplotlib)')

args = arg_parser.parse_args()

universe_db = args.universe
trace_id = int(args.traceid)

sample_percent = float(args.sample_percent)
assert sample_percent > 0 and sample_percent <= 100

#------------------------------------------------------------------------------
# read and plot each generation fitness distribution
#------------------------------------------------------------------------------

kAlpha = 0.5

fig, ax = plt.subplots(figsize=(15, 5))
plt.grid(True)

db = sqlite3.connect(universe_db)
db.row_factory = sqlite3.Row

cursor = db.cursor()

cursor.execute(
    '''select count(*)
        from generation
        where trace_id = ?''',
    (trace_id, ))

count = cursor.fetchone()[0]
assert count > 0

def fitnessValues(details):
    if 'compressed_fitness' in details:
        fitness = details['compressed_fitness']
        x = [cfv[0] for cfv in fitness]
        y = [cfv[1] for cfv in fitness]
        return x, y
    elif 'full_fitness' in details:
        y = details['full_fitness']
        return range(len(y)), y

    # we don't have fitness values, aborting
    sys.exit('Fitness values not found')
    
def uncompressFitnessValues(cx, cy):
    assert len(cx) > 0
    assert len(cx) == len(cy)
    ux = []
    uy = []
    for x, y in zip(cx, cy):
        if ux:
            last_x = ux[-1]
            last_y = uy[-1]
            m = (y - last_y) / (x - last_x)
            for ax in range(last_x + 1, x):
                ux.append(ax)
                uy.append(last_y + (ax - last_x) * m)
        ux.append(x)
        uy.append(y)
    return ux, uy
    
def plotGenerationFitness(details, a):
    x, y = fitnessValues(details)
    if args.sorted:
        plt.plot(x, y, color = [1 - a, 0.5, a, kAlpha], linewidth = 1)
    else:
        x, y = uncompressFitnessValues(x, y)
        sns.kdeplot(y, color = [1 - a, 0.5, a, kAlpha], linewidth = 1)

# sampling values
sample_count = count * (sample_percent / 100)
step = 1 / sample_count
current = 0

print(f'Sampling {int(sample_count)} generations out of {count} ...')

cursor.execute(
    '''select details, generation
        from generation
        where trace_id = ?
        order by generation''',
    (trace_id, ))

for generation in cursor:
    n = generation['generation']
    a = n / count
    if a >= current or n == count - 1:
        details = json.loads(generation['details'])
        if details is None:
            # we don't have fitness values, aborting
            sys.exit('Fitness values not found')
        plotGenerationFitness(details, a)
        current += step

if args.output != None:
    print(f'Saving chart to: {args.output}')
    plt.savefig(args.output)

plt.show()
