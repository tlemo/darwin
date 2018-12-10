
## Dec 9, 2018

Added a rudimentary support for batching experiment runs in Darwin Studio. The
"experiment run dialog" allows setting the number of batch runs (default is 1) and
also a `max_generations` limit:

![Fitness Values](docs/images/batch_run_dialog.png)

This has the same effect as repeatedly running the same experiment variation multiple
times, and the result of each run is recorded as a separate `trace` pointing to the same
parent variation.

The aggregated results can be visualized using the new 
[batch_fitness.py](scripts/docs/scripts.md#batch_fitnesspy) script.

## Nov 11, 2018

Initial release
