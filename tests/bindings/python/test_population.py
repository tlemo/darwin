
import unittest
import darwin

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


if __name__ == '__main__':
    unittest.main()
