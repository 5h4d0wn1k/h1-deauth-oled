#!/usr/bin/env python3
"""H1 - Deauth Detector host helper: offline analysis of deauth-rate logs.
Educational/authorized own-lab use only (see README "IMPORTANT").
"""
import argparse
import os
import sys

MOD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, MOD)
from hw_common import DEMO_TAG, read_target


def parse(line):
    # [DEAUTH] 00:11:22:33:44:55 reason=7
    line = line.strip()
    if not line.startswith("[DEAUTH]"):
        return None
    rest = line[len("[DEAUTH]"):].strip()
    mac, _, reason = rest.partition(" reason=")
    return {"mac": mac.strip(), "reason": int(reason or 0)}


def analyze(text):
    rows = [parse(l) for l in text.splitlines()]
    rows = [r for r in rows if r]
    return rows


def classify(count, seconds):
    rate = count / max(seconds, 1)
    if rate >= 20:
        return "CRITICAL"
    if rate >= 5:
        return "WARNING"
    return "NORMAL"


def run_demo():
    sample = read_target("fixtures/capture.log",
                         "[DEAUTH] 00:11:22:33:44:55 reason=7\n"
                         "[DEAUTH] 00:11:22:33:44:55 reason=3\n"
                         "[DEAUTH] 00:11:22:33:44:66 reason=7\n"
                         "[DEAUTH] 00:11:22:33:44:77 reason=1\n")
    rows = analyze(sample)
    print("=== H1 deauth-rate analysis (offline) ===")
    for r in rows:
        print("  %s  reason=%d" % (r["mac"], r["reason"]))
    print("  Seconds: 10  Level: %s" % classify(len(rows), 10))
    print(DEMO_TAG)
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="H1 Deauth Detector - offline capture-log analysis")
    parser.add_argument("--demo", action="store_true",
                        help="run offline demo on fixture/sample data (exit 0)")
    parser.add_argument("--file", help="path to a capture log to analyze")
    args = parser.parse_args(argv)
    if args.demo or not args.file:
        return run_demo()
    rows = analyze(open(args.file).read())
    for r in rows:
        print("%s reason=%d" % (r["mac"], r["reason"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
