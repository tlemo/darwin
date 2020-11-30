
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

        # invalid name (empty)
        with self.assertRaises(RuntimeError):
            experiment.name = ''

        experiment.name = 'final'

        experiment.initialize_population()

        # can't set the name after the experiment is initialized
        with self.assertRaises(RuntimeError):
            experiment.name = 'baz'

        second_population = darwin.Population('cne.lstm')
        second_domain = darwin.Domain('tic_tac_toe')

        # duplicate name
        with self.assertRaises(RuntimeError):
            second_experiment = universe.new_experiment(
                domain=second_domain, population=second_population, name='final')

        second_experiment = universe.new_experiment(
            domain=second_domain, population=second_population, name='baz')

        # ok...
        second_experiment.name = 'new name'

        # duplicate name
        with self.assertRaises(RuntimeError):
            second_experiment.name = 'final'

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

        # after the experiment is disposed, we can reuse the population and domain
        experiment_a = None
        experiment_d = universe.new_experiment(domain, population, name='D')

    def test_experiment_reset(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)
        population = darwin.Population('neat')
        domain = darwin.Domain('unicycle')

        population.size = 5
        experiment_a = universe.new_experiment(domain, population)

        for i in range(3):
            experiment_a.name = f'A{i}' # name must be unique
            experiment_a.initialize_population();
            experiment_a.evaluate_population();
            experiment_a.create_next_generation();
            experiment_a.reset()

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
