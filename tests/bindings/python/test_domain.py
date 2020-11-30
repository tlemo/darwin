
import unittest
import darwin

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


if __name__ == '__main__':
    unittest.main()
