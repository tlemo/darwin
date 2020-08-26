
import unittest
import darwin


class BasicTestCase(unittest.TestCase):
    def test_add(self):
        self.assertEqual(darwin.add(1, 2), 3)


if __name__ == '__main__':
    unittest.main()
