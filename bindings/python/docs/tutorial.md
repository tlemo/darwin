
# Python Bindings for Darwin: Tutorial

[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/tlemo/darwin/master?urlpath=lab/tree/notebooks/tutorial.ipynb)

- [A first example](#a-first-example)
  - [Opening or creating a Darwin universe](#opening-or-creating-a-darwin-universe)
  - [Selecting and configuring a Population](#selecting-and-configuring-a-population)
  - [Selecting and configuring a Domain](#selecting-and-configuring-a-domain)
  - [Creating a new Experiment](#creating-a-new-experiment)
  - [Evolve!](#evolve)
- [darwin.PropertySet](#darwinpropertyset)
- [darwin.GenerationSummary](#darwingenerationsummary)
- [darwin.Trace](#darwintrace)

## A first example

The following example illustrates the key objects and evolution cycle:

```python
    import darwin

    # opens (or creates) a Darwin universe database
    universe = darwin.open_universe('example.darwin')

    # select a population
    population = darwin.Population('neat')
    population.size = 1000

    # select a domain
    domain = darwin.Domain('unicycle')

    # create a new experiment
    experiment = universe.new_experiment(domain, population)

    # basic evolution cycle
    experiment.initialize_population()
    for generation in range(10):
        print(f'Generation {generation} ...')

        summary = experiment.evaluate_population()
        print(f'  best fitness={summary.best_fitness:.3f}')
        print(f'  median fitness={summary.median_fitness:.3f}')
        print(f'  worst fitness={summary.worst_fitness:.3f}')
        print()

        experiment.create_next_generation()
```

Running this piece of code should produce something like this:

```
    Generation 0 ...
      best fitness=0.295
      median fitness=0.014
      worst fitness=0.013

    Generation 1 ...
      best fitness=0.729
      median fitness=0.049
      worst fitness=0.012

    Generation 2 ...
      best fitness=1.600
      median fitness=0.078
      worst fitness=0.012

    ...
```

The rest of the tutorial will break down this example and explain the key parts.

### Opening or creating a Darwin universe

In the Darwin Framework, all the experiment variations and runs are recorded in
a [universe database][2], so one of the first things we need to do is open or
create a darwin.Universe instance:

```python
    # opens (or creates) a Darwin universe database
    universe = darwin.open_universe('example.darwin')
```

A universe may be explicitly closed using `universe.close()`, or using the
`with` statement:

```python
    with darwin.open_universe(path) as universe:
        ...
```

> IMPORTAT: Don't close the universe while there are active experiments using
it. This will likely result in a crash the next time the experiment will try to write
to the database.

### Selecting and configuring a Population

In order to setup an experiment, we need a _Population_, which abstracts a
particular type of Evolutionary Algoritm, ex. Conventional Neuroevolution (CNE)
or NEAT. A population is a fixed length collection of genotypes (solution
recipes)

Here we're instantiating a 'neat' population type and set its size to
`1000` genotypes:

```python
    population = darwin.Population('neat')
    population.size = 1000
```

`available_populations()` returns a list with the names of all available
population types:

```python
>>> darwin.available_populations()
['cgp', 'cne.feedforward', 'cne.full_rnn', 'cne.lstm', 'cne.lstm_lite', 'cne.rnn', 'neat', 'test_population']
```

Besides the size, each population type has a number of configuration knobs, 
exposed through the `.config` attribute, which is a dictionary-like object.
Each Population is created with a sensible set of defaults, although for real
experiments we may want to tweak the values. 

> NOTE: Each Population type has its specific set of configuration knobs.

Finally, a Population, once initialized (`experiment.initialize_population()`),
can be indexed and iterated over (the genotypes in a population are always
sorted in descending fitness order)

```python
genotype = population[0] # this is the genotype with the best fitness value
...
for genotype in population:
    ...
```

### Selecting and configuring a Domain

Domains abstract the problem space and provides the details on how to evaluate
and assign fitness values to each genotype in a Population.

```python
    domain = darwin.Domain('unicycle')
```

The set of available domains can be discovered using `avaiable_domains()`:

```python
>>> darwin.available_domains()
['ballistics', 'car_track', 'cart_pole', 'conquest', 'double_cart_pole', 'drone_follow', 'drone_track', 'drone_vision', 'find_max_value', 'harvester', 'pong', 'test_domain', 'tic_tac_toe', 'unicycle']
```

Similar to Populations, each Domain type exposes a specific set of configuration
knobs through the `config` attribute.

### Creating a new Experiment

Domains and Populations don't know anything about each other, which is how
Darwin Framework can run NxM combinations with only N Domains + M Populations.

The Experiment object encapsulates a particular Domain, Population combination:

```python
    experiment = universe.new_experiment(domain, population)
```

Experiments also have a `.config` attribute, plus a `.core_config` attribute
with even more knobs common to most Populations.

- `experiment.trace` returns the trace of the current run, once the experiment
is initialed (see `darwin.Trace`)

- `experiment.reset()` will terminate the current evolution run, allowing the
configuration values to be changed again.

### Evolve!

The whole point of setting up the _Population_ and the _Domain_ and putting them
together as an _Experiment_, is to search for solutions (models) using an
[_Evolutionary Algorithm_][1].

The general structure of an evolution loop is simple enough:

```python
    experiment.initialize_population() # (1)
    for generation in range(10):
        summary = experiment.evaluate_population() # (2)
        experiment.create_next_generation() # (3)
```

1. Initialize the population with random genotypes. This step also marks the
experiment as "active" - which means that it starts being recorded to the 
universe database. The configuration values can't be changed anymore (this
include Population and Domain configurations)

2. Evaluate the population, using the Domain instance to assign fitness values
to every genotype in the population. `evaluate_population()` returns a
`darwin.GenerationSummary` object.

3. Create a new generation, starting from the current population with the 
fitness values set at step 2 (`evaluate_population()` must be called before `create_next_generation()`)

## darwin.PropertySet

The configuration knobs for Populations, Domains and Experiments are 
dictionary-like objects of type `darwin.PropertySet`:

- `str(property_set)` returns a string representation of all the
values in a property set (useful to discover the set of knobs/values)

- `dir(property_set)` returns a list of all the properties in a property set

- `darwin.PropertySet.to_json()` returns a JSON string representation of the 
property set

- `darwin.PropertySet.from_json(json_string)` will update the property set from
the `json_string`, which is a JSON string.

## darwin.GenerationSummary

`darwin.GenerationSummary` records key values after a Population evaluation:

- `darwin.GenerationSummary.generation`

- `darwin.GenerationSummary.best_fitness`

- `darwin.GenerationSummary.median_fitness`

- `darwin.GenerationSummary.worst_fitness`

- `darwin.GenerationSummary.champion` - best genotype in the generation

- `darwin.GenerationSummary.calibration_fitness` - domain specific scores
    against reference baselines (ex. a random agent)

## darwin.Trace

`darwin.Trace` is a recording of all the generations in the current experiment,
once started. It can be indexed and iterated over, returning the
`GenerationSummary` for all completed generations.


[1]: https://github.com/tlemo/darwin#evolutionary-algorithms-and-neuroevolution
[2]: https://github.com/tlemo/darwin#running-experiments--the-universe-database
