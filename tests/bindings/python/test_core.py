
import unittest
import darwin


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

    def test_bool_attributes(self):
        p = darwin.Population('neat')
        p.config.normalize_input = True
        self.assertEqual(repr(p.config.normalize_input), 'true')
        p.config.normalize_input = False
        self.assertEqual(repr(p.config.normalize_input), 'false')

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
