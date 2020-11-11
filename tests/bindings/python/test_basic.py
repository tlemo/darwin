
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


class PropertySetTestCase(unittest.TestCase):
    def test_dir(self):
        p = darwin.Population('cne.lstm')
        attr_list = dir(p.config)
        self.assertTrue(attr_list)
        self.assertIn('activation_function', attr_list)
        self.assertIn('normalize_input', attr_list)


if __name__ == '__main__':
    unittest.main()
