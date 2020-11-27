
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
            summary = experiment.evaluate_population()
            experiment.create_next_generation()

        # evolution trace
        trace = experiment.trace
        self.assertEqual(trace.size, 5)
        self.assertEqual(trace.size, len(trace))
        self.assertEqual(trace[4].champion.fitness, summary.champion.fitness)
        self.assertEqual(trace[4].champion.fitness, trace[-1].champion.fitness)
        self.assertEqual(trace[0].champion.fitness, trace[-5].champion.fitness)

        with self.assertRaises(IndexError):
            tmp = trace[5]
        with self.assertRaises(IndexError):
            tmp = trace[-6]

        universe.close()

    def test_evaluate_population(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)

        population = darwin.Population('neat')
        domain = darwin.Domain('tic_tac_toe')

        population.size = 10
        experiment = universe.new_experiment(domain, population)

        experiment.initialize_population()
        summary = experiment.evaluate_population()

        self.assertEqual(population.size, 10)
        self.assertEqual(population.size, len(population))

        # generation summary
        self.assertEqual(summary.generation, 0)
        self.assertEqual(summary.best_fitness, summary.champion.fitness)

        # population fitness values
        self.assertGreaterEqual(population[0].fitness, population[-1].fitness)
        self.assertEqual(population[0].fitness, summary.champion.fitness)

        prev_fitness = summary.champion.fitness
        for genotype in population:
            self.assertLessEqual(genotype.fitness, prev_fitness)
            prev_fitness = genotype.fitness

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
