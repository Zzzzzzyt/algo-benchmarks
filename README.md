# Algorithm Benchmarks

Measuring the actual constant behind asymptotic complexity

## Usage

Only linux is supported.

Run `python3 calibrate_tsc.py` to obtain TSC frequency before running main script. You may need to reboot if the script cannot find TSC info in dmesg.

Run `python3 main.py -a -p` to run all benchmarks. Or better, use `sudo python3 main.py -a -P --pin 0` to use realtime scheduling and pin the process to a single core to get more stable results.
