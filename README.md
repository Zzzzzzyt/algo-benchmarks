# Algorithm Benchmarks

Measuring the actual constant behind asymptotic complexity

## Usage

Only linux is supported.

Run `python3 calibrate_tsc.py` to obtain TSC frequency before running main script. You may need to reboot if the script cannot find TSC info in dmesg.

Run `python3 main.py --all-profiles -p` to run all benchmarks.
