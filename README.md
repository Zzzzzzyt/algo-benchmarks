# Algorithm Benchmarks

Measuring the actual constant behind asymptotic complexity

## Usage

This script is intended to run on Linux, but Windows is also supported (but not recommended).

Run `python3 calibrate_tsc.py` to obtain TSC frequency before running main script. You may need to reboot if the script cannot find TSC info in dmesg.

Run `python3 main.py --all-profiles -p` to run all benchmarks.
