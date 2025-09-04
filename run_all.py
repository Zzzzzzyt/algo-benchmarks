import os
import json
from main import run
import argparse

parser = argparse.ArgumentParser(description="Run benchmarks")
parser.add_argument("--device", type=str, help="Device name to use in results", required=False, default=None)
parser.add_argument("--rerun", action="store_true", help="Do not skip existing tests", required=False, default=False)
args = parser.parse_args()

os.chdir(os.path.dirname(os.path.abspath(__file__)))
profiles = json.load(open("profiles.json", "r"))
results_index = []

if args.device is None:
    device = input("Enter device name: ").strip()
else:
    device = args.device

for profile_name, profile in profiles.items():
    output_file = f"results/{device}_{profile_name.replace(' ', '_')}.json"
    print()
    run(profile, "benchmarks", output_file, args.rerun)
    results_index.append({"name": f"{device} {profile_name}", "path": output_file})

json.dump(results_index, open("results_index.json", "w"))
