
import unittest
import darwin

import darwin_test_utils


class UniverseTestCase(unittest.TestCase):
    def test_create(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.create_universe(path)
        self.assertFalse(universe.closed)
        self.assertEqual(universe.path, path)
        universe.close()
        self.assertTrue(universe.closed)

    def test_opern_or_create(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')
        universe = darwin.open_universe(path)
        self.assertFalse(universe.closed)
        self.assertEqual(universe.path, path)
        universe.close()
        self.assertTrue(universe.closed)

    def test_context_manager(self):
        path = darwin_test_utils.reserve_universe('python_bindings.darwin')

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


if __name__ == '__main__':
    unittest.main()
