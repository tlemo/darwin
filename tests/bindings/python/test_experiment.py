
import unittest
import darwin

import darwin_test_utils


class ExperimentTestCase(unittest.TestCase):
    def test_new_experiment(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        with darwin.create_universe(path) as universe:
            p = darwin.Population('neat')
            d = darwin.Domain('unicycle')
            exp = universe.new_experiment(population=p, domain=d)
            self.assertRegex(repr(exp), p.name)
            self.assertRegex(repr(exp), d.name)
            self.assertIs(exp.population, p)
            self.assertIs(exp.domain, d)
            self.assertIs(exp.universe, universe)

    def test_experiment_name(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)
        population = darwin.Population('neat')
        domain = darwin.Domain('unicycle')

        experiment = universe.new_experiment(domain, population, name='foo')
        self.assertEqual(experiment.name, 'foo')
        self.assertRegex(repr(experiment), "'foo'")

        experiment.name = ' a new  name  with  lots of  spaces! '
        self.assertRegex(repr(experiment), "' a new  name  with  lots of  spaces! '")

        experiment.name = None
        self.assertRegex(repr(experiment), 'Unnamed')

        with self.assertRaises(RuntimeError):
            experiment.name = ''

        experiment.initialize_population()

        with self.assertRaises(RuntimeError):
            experiment.name = 'baz'

    def test_reuse_checks(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)
        population = darwin.Population('neat')
        domain = darwin.Domain('unicycle')

        experiment_a = universe.new_experiment(domain, population, name='A')

        # currently, once a population or a domain _instance_ is used by an experiment,
        # it cannot be reused in other experiments

        new_population = darwin.Population('neat')
        with self.assertRaises(RuntimeError):
            experiment_b = universe.new_experiment(domain, new_population, name='B')

        new_domain = darwin.Domain('unicycle')
        with self.assertRaises(RuntimeError):
            experiment_c = universe.new_experiment(new_domain, population, name='C')

    def test_sealed_state(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)
        population = darwin.Population('neat')
        domain = darwin.Domain('unicycle')
        experiment = universe.new_experiment(domain, population)

        # changes are ok
        population.size = 10
        population.config.normalize_input = False
        domain.config.gravity = 10.5
        experiment.config.save_genealogy = True
        experiment.core_config.mutation_std_dev = 0.5

        d_config = domain.config
        d_config.max_torque = 100

        # initializing the population will seal all configuration values
        experiment.initialize_population()

        # shouldn't be able to change sealed state
        with self.assertRaises(RuntimeError):
            population.size = 0
        with self.assertRaises(RuntimeError):
            population.config.normalize_input = True
        with self.assertRaises(RuntimeError):
            domain.config.gravity = 0
        with self.assertRaises(RuntimeError):
            experiment.config.save_genealogy = False
        with self.assertRaises(RuntimeError):
            experiment.core_config.mutation_std_dev = 0
        with self.assertRaises(RuntimeError):
            d_config.max_torque = 0

        # make sure the values were not changed
        self.assertEqual(population.size, 10)
        self.assertEqual(population.config.normalize_input.auto, False)
        self.assertEqual(domain.config.gravity.auto, 10.5)
        self.assertEqual(experiment.config.save_genealogy.auto, True)
        self.assertEqual(experiment.core_config.mutation_std_dev.auto, 0.5)
        self.assertEqual(d_config.max_torque.auto, 100)

        # resetting the experiment allows configuration changes again
        experiment.reset()

        population.size = 1
        population.config.normalize_input = 'true'
        domain.config.gravity = 0.1
        experiment.config.save_genealogy = False
        experiment.core_config.mutation_std_dev = '0.1'
        d_config.max_torque = 1.5

        universe.close()


if __name__ == '__main__':
    unittest.main()
