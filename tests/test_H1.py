import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "host"))
import h1_cli as m


class TestParse(unittest.TestCase):
    def test_parse_deauth(self):
        line = "[DEAUTH] 00:11:22:33:44:55 reason=7"
        r = m.parse(line)
        self.assertEqual(r["mac"], "00:11:22:33:44:55")
        self.assertEqual(r["reason"], 7)

    def test_parse_ignores_non_deauth(self):
        self.assertIsNone(m.parse("[FOO] x"))
        self.assertIsNone(m.parse("[DISASSOC] aa reason=3"))

    def test_classify_levels(self):
        self.assertEqual(m.classify(2, 1), "NORMAL")
        self.assertEqual(m.classify(8, 1), "WARNING")
        self.assertEqual(m.classify(20, 1), "CRITICAL")


if __name__ == "__main__":
    unittest.main()
