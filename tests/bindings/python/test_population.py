
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

    def test_genotype_and_brain(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)

        population = darwin.Population('cne.feedforward')
        population.size = 10

        domain = darwin.Domain('tic_tac_toe')
        domain.config.ann_type = 'value'

        experiment = universe.new_experiment(domain, population)
        experiment.initialize_population()

        genotype = population[0].clone()
        self.assertEqual(genotype.to_json(), population[0].to_json())

        brain = genotype.grow()
        for index in range(9):
            brain.set_input(index, 0)
        brain.think()
        board_value = brain.output(index=0)

    def test_genotype_lifetime(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)

        population = darwin.Population('cne.feedforward')
        domain = darwin.Domain('tic_tac_toe')

        population.size = 10

        experiment = universe.new_experiment(domain, population)

        experiment.initialize_population()

        genotype = population[0]

        json_checkpoint_1 = genotype.to_json()

        experiment.evaluate_population()

        # this is fine, although our genotype may be ranked to a different index now
        json_checkpoint_2 = genotype.to_json()

        experiment.create_next_generation()

        # the old genotype is no longer owned by the population
        json_checkpoint_3 = genotype.to_json()

        experiment.reset()

        # the old population is no longer owned by the experiment
        json_checkpoint_4 = genotype.to_json()

        self.assertEqual(json_checkpoint_1, json_checkpoint_2)
        self.assertEqual(json_checkpoint_1, json_checkpoint_3)
        self.assertEqual(json_checkpoint_1, json_checkpoint_4)

    def test_config_lifetime(self):
        p = darwin.Population('neat')
        config = p.config
        p = None
        self.assertTrue(repr(config))


if __name__ == '__main__':
    unittest.main()
