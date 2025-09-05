import os
import json
from main import run
import argparse

parser = argparse.ArgumentParser(description="Run benchmarks for all profiles")
parser.add_argument(
    "--results_index", type=str, help="Path to results index file", required=False, default="results_index.json"
)
parser.add_argument("--device", type=str, help="Device name to use in results", required=False, default=None)
parser.add_argument("--rerun", action="store_true", help="Do not skip existing tests", required=False, default=False)
parser.add_argument("--dry-run", action="store_true", help="Doesn't actually run tests", required=False, default=False)
args = parser.parse_args()

os.chdir(os.path.dirname(os.path.abspath(__file__)))
profiles = json.load(open("profiles.json", "r"))

if os.path.exists("results") is False:
    os.mkdir("results")

if os.path.exists("results_index.json"):
    results_index = json.load(open("results_index.json", "r"))
else:
    results_index = []

if args.device is None:
    device = input("Enter device name: ").strip()
else:
    device = args.device

for profile_name, profile in profiles.items():
    output_file = f"results/{device}_{profile_name.replace(' ', '_')}.json"
    print()
    run(profile, "benchmarks", output_file, args.rerun, args.dry_run)
    results_index = [entry for entry in results_index if entry["path"] != output_file]
    results_index.append({"name": f"{device} {profile_name}", "path": output_file})

results_index.sort(key=lambda x: x["name"])

json.dump(results_index, open("results_index.json", "w"))
