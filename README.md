# Algorithm Benchmarks

Measuring the actual constant behind asymptotic complexity

## Usage

Only linux is supported.

Install `requirements.txt`.

Run `python3 calibrate_tsc.py` to obtain TSC frequency info `tsc_freq.txt` before running main script. If the script cannot find TSC info in dmesg, It will attempt to obtain TSC frequency from cpuid or measure it by itself.

Run `python3 main.py -a -p` to run all benchmarks. Or better, use `sudo python3 main.py -a -pp --pin 0` to use realtime scheduling and pin the process to a single core to get more stable results.

If `comment.txt` is present in repo directory, its content will be added as a comment to the results by default.
