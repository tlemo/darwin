
import unittest
import darwin
import os


def reserve_universe(filename):
    """deletes the file if it exists, then returns the full path"""
    darwin_home_path = os.getenv('DARWIN_HOME_PATH')
    if darwin_home_path is None:
        raise RuntimeError('DARWIN_HOME_PATH must be set before running the tests')
    full_path = os.path.abspath(os.path.join(darwin_home_path, filename))
    if os.path.exists(full_path):
        os.remove(full_path)
    return full_path


class UniverseTestCase(unittest.TestCase):
    def test_create(self):
        path = reserve_universe('create_universe.darwin')
        universe = darwin.create_universe(path)
        self.assertFalse(universe.closed)
        self.assertEqual(universe.path, path)
        universe.close()
        self.assertTrue(universe.closed)

    def test_context_manager(self):
        path = reserve_universe('ctx_manager.darwin')

        # create the universe
        with darwin.create_universe(path):
            # nothing to do, we just created a new universe
            pass

        # open the universe, using the context manager interface
        with darwin.open_universe(path) as universe:
            self.assertFalse(universe.closed)
            self.assertEqual(universe.path, path)
            self.assertTrue(str(universe).startswith('<darwin.Universe path='))

        self.assertTrue(universe.closed)


class ConfigTestCase(unittest.TestCase):
    def test_available_domains(self):
        domains = darwin.available_domains()
        self.assertTrue(domains)
        for name in domains:
            domain = darwin.Domain(name)
            self.assertEqual(name, domain.name)
            self.assertIsNotNone(domain.config)

    def test_available_populations(self):
        populations = darwin.available_populations()
        self.assertTrue(populations)
        for name in populations:
            population = darwin.Population(name)
            self.assertEqual(name, population.name)
            self.assertIsNotNone(population.config)


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

    def test_config_lifetime(self):
        p = darwin.Population('neat')
        config = p.config
        p = None
        self.assertTrue(repr(config))


class DomainTestCase(unittest.TestCase):
    def test_config_assignment(self):
        d1 = darwin.Domain('pong')
        d2 = darwin.Domain('tic_tac_toe')

        # this is ok, we can bind to config instances
        c = d1.config
        c = d2.config

        # ... but we can't set the config
        with self.assertRaises(AttributeError):
            d1.config = d2.config

        # ... even self assignments
        with self.assertRaises(AttributeError):
            d1.config = d1.config

        # ... or setting to None
        with self.assertRaises(AttributeError):
            d1.config = None

    def test_config_lifetime(self):
        d = darwin.Domain('conquest')
        config = d.config
        d = None
        self.assertTrue(repr(config))


class ExperimentTestCase(unittest.TestCase):
    def test_new_experiment(self):
        path = reserve_universe('new_experiment.darwin')
        with darwin.create_universe(path) as universe:
            p = darwin.Population('neat')
            d = darwin.Domain('unicycle')
            exp = universe.new_experiment(population=p, domain=d)
            self.assertRegex(repr(exp), p.name)
            self.assertRegex(repr(exp), d.name)
            self.assertIs(exp.population, p)
            self.assertIs(exp.domain, d)
            self.assertIs(exp.universe, universe)


class PropertySetTestCase(unittest.TestCase):
    def test_dir(self):
        p = darwin.Population('cne.lstm')
        attr_list = dir(p.config)
        self.assertTrue(attr_list)
        self.assertIn('activation_function', attr_list)
        self.assertIn('normalize_input', attr_list)

    def test_get_attributes(self):
        p = darwin.Population('cne.lstm')
        self.assertEqual(repr(p.config.activation_function), 'tanh')
        self.assertEqual(repr(p.config.normalize_input), 'false')

        # trying to get an nonexistent property
        with self.assertRaises(RuntimeError):
            value = p.config.nonexistent_property

    def test_cast_attributes(self):
        d = darwin.Domain('conquest')
        self.assertEqual(int(d.config.int_unit_scale), 10)
        self.assertEqual(float(d.config.points_draw), 0.4)
        self.assertEqual(repr(d.config.board), 'hexagon')
        self.assertEqual(str(d.config.board), 'hexagon')

        # invalid cast
        with self.assertRaises(RuntimeError):
            value = int(d.config.board)

    def test_property_lifetime(self):
        d = darwin.Domain('conquest')
        property = d.config.board
        d = None
        self.assertEqual(str(property), 'hexagon')

    def test_set_attributes(self):
        p = darwin.Population('cne.lstm')
        p.config.activation_function = 'relu'
        p.config.hidden_layers = '{ 10, 20, 5, 1, 100 }';
        self.assertEqual(repr(p.config.activation_function), 'relu')
        self.assertEqual(repr(p.config.hidden_layers), '{ 10, 20, 5, 1, 100 }')

        # we support simple conversions
        p.config.mutation_chance = 0.15
        self.assertEqual(float(p.config.mutation_chance), 0.15)

        # trying to set an nonexistent property
        with self.assertRaises(RuntimeError):
            p.config.mutation_chance_foo = '0.1'

        # invalid value syntax
        with self.assertRaises(RuntimeError):
            p.config.mutation_chance = '#0.15'

        # setting to a unkown enum value
        with self.assertRaises(RuntimeError):
            p.config.activation_function = 'relu_blah'

        # we have a check to prevent accidental conversions
        with self.assertRaises(RuntimeError):
            p.config.hidden_layers = { 10, 20, 5, 1, 100 };

    def test_property_assignment(self):
        p = darwin.Population('cne.lstm')
        value = p.config.hidden_layers
        p.config.hidden_layers = value

    def test_property_attributes(self):
        d = darwin.Domain('conquest')
        for prop_name in dir(d.config):
            prop = d.config.__getattr__(prop_name)
            self.assertEqual(prop.name, prop_name)
            self.assertTrue(prop.description)
            self.assertTrue(prop.default_value)

    def test_variant_property_sets(self):
        d = darwin.Domain('tic_tac_toe')
        config = d.config

        # not all the properties have sub-variants
        with self.assertRaises(RuntimeError):
            dummy = config.ann_type.variant

        d = None # stress the ownership/lifetimes management
        prop = config.tournament_type
        config.tournament_type = 'swiss'
        swiss_tournament = prop.variant
        config.tournament_type = 'simple'
        simple_tournament = prop.variant
        prop = None # stress the ownership/lifetimes management
        config.tournament_type.variant.eval_games = 123
        config = None # stress the ownership/lifetimes management

        self.assertEqual(int(swiss_tournament.rounds), 20)
        self.assertEqual(int(simple_tournament.eval_games), 123)


if __name__ == '__main__':
    unittest.main()
