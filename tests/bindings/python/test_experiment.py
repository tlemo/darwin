
import unittest
import darwin

import darwin_test_utils


class ExperimentTestCase(unittest.TestCase):
    def test_new_experiment(self):
        path = darwin_test_utils.reserve_universe('new_experiment.darwin')
        with darwin.create_universe(path) as universe:
            p = darwin.Population('neat')
            d = darwin.Domain('unicycle')
            exp = universe.new_experiment(population=p, domain=d)
            self.assertRegex(repr(exp), p.name)
            self.assertRegex(repr(exp), d.name)
            self.assertIs(exp.population, p)
            self.assertIs(exp.domain, d)
            self.assertIs(exp.universe, universe)


if __name__ == '__main__':
    unittest.main()
