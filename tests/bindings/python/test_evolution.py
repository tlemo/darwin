
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


if __name__ == '__main__':
    unittest.main()
