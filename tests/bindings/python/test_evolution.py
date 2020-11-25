
import unittest
import darwin

import darwin_test_utils


class EvolutionTestCase(unittest.TestCase):
    def test_basic_evolution(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)

        population = darwin.Population('neat')
        population.size = 10

        domain = darwin.Domain('unicycle')

        experiment = universe.new_experiment(domain, population)

        # basic evolution cycle
        experiment.initialize_population()
        for generation in range(5):
          experiment.evaluate_population()
          experiment.create_next_generation()

        universe.close()

    def test_generation_summary(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)

        population = darwin.Population('neat')
        domain = darwin.Domain('tic_tac_toe')

        population.size = 10
        experiment = universe.new_experiment(domain, population)

        experiment.initialize_population()
        summary = experiment.evaluate_population()

        self.assertEqual(summary.generation, 0)
        self.assertEqual(summary.best_fitness, summary.champion.fitness)

        universe.close()

    def test_reinitialize_population(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)

        population = darwin.Population('neat')
        population.size = 10

        domain = darwin.Domain('unicycle')

        experiment = universe.new_experiment(domain, population)

        # multiple runs of the same experiment variation
        for evolution_run in range(3):
            # reinitialize the population (which would result in a new evolution trace)
            experiment.initialize_population()
            for generation in range(2):
              experiment.evaluate_population()
              experiment.create_next_generation()

        universe.close()


if __name__ == '__main__':
    unittest.main()
