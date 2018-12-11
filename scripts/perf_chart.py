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

# basic time-per-generation chart

import sqlite3
import json
import argparse
import matplotlib.pyplot as plt

#------------------------------------------------------------------------------
# command line parsing
#------------------------------------------------------------------------------

arg_parser = argparse.ArgumentParser(
    description = 'Runtime perf plotting tool', allow_abbrev = False)

arg_parser.add_argument('-u', '--universe',
    required = True, help = 'Darwin universe database')
arg_parser.add_argument('-t', '--traceid',
    required = True, help = 'Evolution Trace ID')
arg_parser.add_argument('-tb', '--base_traceid',
    required = False, help = 'Evolution Trace ID')
arg_parser.add_argument('-o', '--output',
    help = 'Save chart to the specified file (in a format supported by matplotlib)')
arg_parser.add_argument('--all-generations',
    help = "Don't limit the chart to the shared generations only", action='store_true')

args = arg_parser.parse_args()

universe_db = args.universe
trace_id = int(args.traceid)

#------------------------------------------------------------------------------
# read the universe objects
#------------------------------------------------------------------------------

db = sqlite3.connect(universe_db)
db.row_factory = sqlite3.Row

def generationProfileValues(trace_id):
    cursor = db.cursor()
    cursor.execute(
        '''select profile
            from generation
            where trace_id = ?
            order by generation''',
        (trace_id, ))
    values = []
    for generation in cursor:
        profile = json.loads(generation['profile'])
        values.append(profile['elapsed'])
    return values
    
# read the profile values
profile = generationProfileValues(trace_id)

base = None
if args.base_traceid is not None:
    base_trace_id = int(args.base_traceid)
    base = generationProfileValues(base_trace_id)
    
    # limit to the shared generations by default
    if not args.all_generations:
        common = min(len(base), len(profile))
        base = base[:common]
        profile = profile[:common]

#------------------------------------------------------------------------------
# plot per-generation timing values
#------------------------------------------------------------------------------

fig, ax = plt.subplots(figsize=(15, 5))

kAlpha = 0.7

plt.plot(
    profile,
    color = [0, 0, 1, kAlpha],
    linewidth = 1,
    label = "Profile")

if base is not None:
    plt.plot(
        base,
        color = [0.3, 0.3, 0.3, kAlpha],
        linewidth = 1,
        label = "Base")

plt.grid(True)
plt.tight_layout()
ax.legend(loc='upper left')

if args.output is not None:
    print(f'Saving chart to: {args.output}')
    plt.savefig(args.output)

plt.show()
