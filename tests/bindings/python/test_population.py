
import unittest
import darwin

import darwin_test_utils


class PopulationTestCase(unittest.TestCase):
    def test_config_assignment(self):
        p1 = darwin.Population('cne.lstm')
        p2 = darwin.Population('neat')

        # this is ok, we can bind to config instances
        c = p1.config
        c = p2.config

        # ... but we can't set the config
        with self.assertRaises(AttributeError):
            p1.config = p2.config

        # ... even self assignments
        with self.assertRaises(AttributeError):
            p1.config = p1.config

        # ... or setting to None
        with self.assertRaises(AttributeError):
            p1.config = None

    def test_size(self):
        p = darwin.Population('neat')
        self.assertGreater(p.size, 0);
        p.size = 1 # smallest valid value

        # invalid size value
        with self.assertRaises(RuntimeError):
            p.size = 0

    def test_indexing(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)

        population = darwin.Population('cne.feedforward')
        domain = darwin.Domain('tic_tac_toe')

        population.size = 10

        experiment = universe.new_experiment(domain, population)

        # can't index into an uninitialized population
        with self.assertRaises(RuntimeError):
            genotype = population[0]

        experiment.initialize_population()

        # correct indexing
        genotype = population[0]
        genotype = population[9]
        genotype = population[-1]
        genotype = population[-10]

        # out of bounds
        with self.assertRaises(IndexError):
            genotype = population[10]
        with self.assertRaises(IndexError):
            genotype = population[-11]

    def test_config_lifetime(self):
        p = darwin.Population('neat')
        config = p.config
        p = None
        self.assertTrue(repr(config))


if __name__ == '__main__':
    unittest.main()
