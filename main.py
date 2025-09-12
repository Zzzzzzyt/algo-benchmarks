import math
import os
import argparse
import json
import re
import sys
import subprocess
import shutil
import random
import hashlib
import time


def colorize(text, color):
    colors = {
        "red": "\033[91m",
        "green": "\033[92m",
        "yellow": "\033[93m",
        "blue": "\033[94m",
        "magenta": "\033[95m",
        "cyan": "\033[96m",
        "white": "\033[97m",
        "gray": "\033[90m",
        "endc": "\033[0m",
    }
    return f"{colors.get(color, '')}{text}{colors['endc'] if color in colors else ''}"


def get_complexity_fn(complexity):
    if complexity == "O(n)":
        return lambda n: n
    elif complexity == "O(n^2)":
        return lambda n: n**2
    elif complexity == "O(n^3)":
        return lambda n: n**3
    elif complexity == "O(logn)":
        return lambda n: math.log2(n)
    elif complexity == "O(nlogn)":
        return lambda n: n * math.log2(n)
    elif complexity == "O(1)":
        return lambda n: 1
    elif complexity == "O(sqrt(n))":
        return lambda n: math.sqrt(n)
    elif complexity == "O(sqrt(n)/logn)":
        return lambda n: math.sqrt(n) / math.log2(n)
    else:
        raise ValueError(f"Unknown complexity: {complexity}")


def generator(
    lower_bound=1,
    upper_bound=10**8,
    power_of_two=False,
    micro_repeats=False,
    estimated_constant=1,
    complexity="O(n)",
    max_repeats=1_000_000,
    min_runtime=10_000_000,
):
    if power_of_two:
        ns = [2**i for i in range(64)]
    else:
        ns = [
            1,
            2,
            3,
            4,
            6,
            8,
            10,
            16,
            20,
            32,
            40,
            50,
            64,
            100,
            128,
            200,
            256,
            350,
            512,
            700,
            1000,
            1024,
            1500,
            2048,
            3000,
            4096,
            5000,
            8192,
            10000,
            16384,
            20000,
            32768,
            50000,
            65536,
            100000,
            131072,
            200000,
            262144,
            400000,
            524288,
            700000,
            1000000,
            1048576,
            1500000,
            2097152,
            3000000,
            4194304,
            5000000,
            8388608,
            10000000,
            16777216,
            20000000,
            33554432,
            50000000,
            67108864,
            100000000,
            134217728,
            200000000,
            268435456,
            400000000,
            536870912,
            700000000,
            1000000000,
            1073741824,
            1500000000,
            2147483648,
            3000000000,
            4294967296,
            6000000000,
            8589934592,
            13000000000,
            17179869184,
            25000000000,
            34359738368,
            50000000000,
            68719476736,
            100000000000,
            137438953472,
            200000000000,
            274877906944,
            400000000000,
            549755813888,
            800000000000,
            1099511627776,
            1500000000000,
            2199023255552,
            3000000000000,
            4398046511104,
            6000000000000,
            8796093022208,
            12000000000000,
            17592186044416,
            25000000000000,
            35184372088832,
            50000000000000,
            70368744177664,
            100000000000000,
            140737488355328,
            200000000000000,
            281474976710656,
            400000000000000,
            562949953421312,
            800000000000000,
            1125899906842624,
            1500000000000000,
            2251799813685248,
            3000000000000000,
            4503599627370496,
            7000000000000000,
            9007199254740992,
            13000000000000000,
            18014398509481984,
            25000000000000000,
            36028797018963968,
            50000000000000000,
            72057594037927936,
            100000000000000000,
            144115188075855872,
            200000000000000000,
            288230376151711744,
            400000000000000000,
            576460752303423488,
            800000000000000000,
            1152921504606846976,
            1800000000000000000,
            2305843009213693952,
            3000000000000000000,
            4611686018427387904,
            7000000000000000000,
            9223372036854775808,
        ]

    ns.sort()
    ret = []
    if micro_repeats:
        complexity_fn = get_complexity_fn(complexity)
    for n in ns:
        if n < lower_bound or n > upper_bound:
            continue
        if micro_repeats:
            micro_repeats = max(1, min(max_repeats, math.ceil(min_runtime / complexity_fn(n) / estimated_constant)))
            ret.append(
                {
                    "defs": {
                        "BENCHMARK_N": n,
                        "BENCHMARK_MICRO_REPEATS": micro_repeats,
                    }
                }
            )
        else:
            ret.append({"defs": {"BENCHMARK_N": n}})
    return ret


def compile_source(source, profile, defs={}, high_priority=False):
    if high_priority:
        defs["BENCHMARK_HIGH_PRIORITY"] = 1

    defs["BENCHMARK_TSC_FREQ"] = f"{tsc_freq:.10f}"

    if os.path.exists("temp"):
        shutil.rmtree("temp")
    os.makedirs("temp", exist_ok=True)
    source = open(source["path"], "r").read()

    source = source.replace('#include "utils.h"', open("utils.h", "r").read())

    source_path = "temp/source.cpp"
    output_path = "temp/output"
    if sys.platform == "win32":
        output_path += ".exe"

    open(source_path, "w").write(source)

    defines = []
    for k, v in defs.items():
        v = str(v)
        defines.append(f"-D{k}={v}")

    compile_command = profile["build_command"].format(output=output_path, source_path=source_path, defines=" ".join(defines))
    print(colorize(compile_command, "magenta"))

    subprocess.run(compile_command, shell=True, check=True)
    if not os.path.exists(output_path):
        print(colorize(f"Compilation failed with command: {compile_command}", "red"))
        exit(1)

    return output_path


def handle_simple_test(testid, test, line, input_data):
    values = line.split(":")[1].strip().split(" ")
    cur = {}
    for i, value in enumerate(values):
        cur[test["template"][i]] = float(value)
    return cur


def process_simple_test(testid, test):
    stat = {}
    for sample in test["data"]:
        n = sample["n"]
        if n not in stat:
            stat[n] = []
        t = sample["time_ns"]
        if "micro_repeats" in sample:
            t /= sample["micro_repeats"]
        stat[n].append(t)

    complexity_fn = get_complexity_fn(test["complexity"])

    # Compute statistics for each key
    stats = []
    for n, values in stat.items():
        values.sort()
        raw_values = values.copy()
        remove_outliers = test.get("max_outlier", max(1, round(len(values) / 6)))

        mean = sum(values) / len(values)
        stddev = math.sqrt(sum((x - mean) ** 2 for x in values) / (len(values) - 1))

        removed = []
        for _ in range(remove_outliers):
            if values[-1] > values[len(values) // 2] + 3 * stddev:
                removed.append(values[-1])
                values.pop()
                mean = sum(values) / len(values)
                stddev = math.sqrt(sum((x - mean) ** 2 for x in values) / (len(values) - 1))
            else:
                break

        removed = removed[::-1]

        if len(removed) > 0:
            print(colorize(f"Removed {len(removed)} outliers from {testid} n={n}", "yellow"))
            print(colorize(f"  Raw values: {raw_values}", "gray"))
            print(colorize(f"  Removed values: {removed}", "gray"))

        complexity = complexity_fn(n)
        mean_c = mean / complexity
        stddev_c = stddev / complexity

        stats.append(
            {
                "n": n,
                "mean": mean,
                "stddev": stddev,
                "mean_c": mean_c,
                "stddev_c": stddev_c,
                "min_c": min(values) / complexity,
                "max_c": max(values) / complexity,
                "samples": len(values),
                "min": min(values),
                "max": max(values),
                "complexity": complexity,
                "raw_values": raw_values,
            }
        )

    maxc = 0
    for sample in stats:
        if test["practical_lower_bound"] <= sample["n"] <= test["practical_upper_bound"]:
            maxc = max(maxc, sample["mean"] / complexity_fn(sample["n"]))

    test["max_c"] = maxc
    test["stats"] = stats
    del test["data"]
    return test


def run_source(source, profile, dry_run=False, high_priority=False):
    global tests

    ret = {}
    inputs = []
    if source["input"]["type"] == "generator":
        inputs = generator(**source["input"]["params"])
    else:
        raise ValueError(f"Unsupported input type: {source['input']['type']}")

    for input_data in inputs:
        if dry_run:
            print(colorize(f"Would compile {source['path']} with profile {profile['name']}", "magenta"))
            print(colorize("with defs: " + str(input_data.get("defs", {})), "magenta"))
            for testid in source["tests"]:
                for repeat in range(source["repeats"]):
                    test = tests[testid]
                    if testid not in ret:
                        ret[testid] = test.copy()
                        ret[testid]["data"] = []
                    if test["type"] == "simple":
                        fake_input = f"{testid}:\t"
                        for field in test["template"]:
                            if field == "n":
                                fake_input += f"{input_data['defs']['BENCHMARK_N']} "
                            else:
                                fake_input += f"{random.randint(1000, 100000)} "
                        ret[testid]["data"].append(
                            handle_simple_test(
                                testid,
                                test,
                                fake_input,
                                input_data,
                            )
                        )
        else:
            output_path = compile_source(source, profile, input_data.get("defs", {}), high_priority=high_priority)
            for repeat in range(source["repeats"]):
                p = subprocess.run(
                    output_path,
                    check=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                )
                print(colorize(p.stdout.decode().strip(), "gray"))
                if p.returncode != 0:
                    print(colorize(f"Error running {source['path']} with input {input_data}:", "red"))
                    print(colorize(p.stderr.decode().strip(), "red"))
                    exit(1)
                for line in p.stdout.decode().splitlines():
                    testid = line.split(":")[0].strip()
                    test = tests[testid]
                    if testid not in ret:
                        ret[testid] = test.copy()
                        ret[testid]["data"] = []
                    if test["type"] == "simple":
                        ret[testid]["data"].append(handle_simple_test(testid, test, line, input_data))
    return ret


def collect_environment():
    global tsc_freq

    env = {
        "platform": sys.platform,
        "python_version": sys.version,
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S", time.gmtime()),
        "tsc_freq": tsc_freq,
    }
    try:
        # Collect g++ version info
        gpp_proc = subprocess.run(["g++", "-v"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        env["g++"] = gpp_proc.stdout if gpp_proc.stdout else gpp_proc.stderr
    except Exception as e:
        env["g++"] = f"Error: {e}"

    try:
        # Collect cache info
        lscpu_proc = subprocess.run(["lscpu", "-C"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        env["cacheinfo"] = lscpu_proc.stdout if lscpu_proc.stdout else lscpu_proc.stderr
    except Exception as e:
        env["cacheinfo"] = f"Error: {e}"

    try:
        # Collect lscpu info
        lscpu_proc = subprocess.run(["lscpu"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        env["cpuinfo"] = lscpu_proc.stdout if lscpu_proc.stdout else lscpu_proc.stderr
    except Exception as e:
        env["cpuinfo"] = f"Error: {e}"

    try:
        # Collect free memory info
        free_proc = subprocess.run(["free", "-h"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        env["meminfo"] = free_proc.stdout if free_proc.stdout else free_proc.stderr
    except Exception as e:
        env["meminfo"] = f"Error: {e}"

    return env


def hash_obj(obj):
    return hashlib.md5(json.dumps(obj, sort_keys=True).encode()).hexdigest()


def run(profile, source_path, output_file, rerun=False, dry_run=False, high_priority=False, test_filter=None):
    global tsc_freq
    if os.path.exists("tsc_freq.txt"):
        tsc_freq = float(open("tsc_freq.txt", "r").read().strip())
    else:
        print(colorize("TSC frequency file tsc_freq.txt not found. Please run calibrate_tsc script.", "red"))
        exit(1)

    print(colorize(f"Using profile: {profile['name']}", "green"))
    random.seed(42)

    global tests
    global results
    global old_results

    tests = {}
    results = {}
    sources = []

    def proc_cfg(dirpath, cfg_path):
        nonlocal sources
        global tests

        cfg = json.load(open(cfg_path, "r", encoding="utf-8"))
        source_hash = []
        source_files = []
        for source in cfg["sources"]:
            source_config_hash = hash_obj(source)
            if test_filter:
                source["tests"] = [testid for testid in cfg["tests"].keys() if re.match(test_filter, testid)]
            else:
                source["tests"] = list(cfg["tests"].keys())
            path = source["path"] = os.path.join(dirpath, source["path"])
            source_hash.append(
                [
                    hashlib.md5(open(path, "rb").read().replace(b"\r", b"")).hexdigest(),
                    source_config_hash,
                ]
            )
            source_files.append(path)
            sources.append(source)

        source_hash = hash_obj(source_hash)

        for testid, test in cfg["tests"].items():
            if test_filter and (re.match(test_filter, testid) is None):
                continue
            test["source_files"] = source_files
            test["source_hash"] = source_hash
            test["test_hash"] = hash_obj(test)
            tests[testid] = test

    if os.path.isfile(source_path):
        proc_cfg(os.path.dirname(source_path), source_path)
    else:
        for dirpath, dirnames, filenames in os.walk(source_path):
            for file in filenames:
                if file.endswith(".json"):
                    proc_cfg(dirpath, os.path.join(dirpath, file))

    print(colorize(f"Found {len(sources)} source files and {len(tests)} tests.", "green"))

    old_results = {}
    if (not rerun) and os.path.exists(output_file):
        old_results_file = json.load(open(output_file, "r", encoding="utf-8"))
        old_profile_hash = hash_obj(old_results_file.get("profile", {}))
        current_profile_hash = hash_obj(profile)
        if old_profile_hash == current_profile_hash:
            for k, v in old_results_file.get("results", {}).items():
                old_test_hash = v.get("test_hash", -1)
                if k not in tests:
                    continue
                current_test_hash = tests[k].get("test_hash", -1)
                if old_test_hash != current_test_hash:
                    continue
                old_results[k] = v
        print(colorize(f"Loaded {len(old_results)} existing test results from {output_file}", "green"))

    unused_sources = set()
    for source in sources:
        flag = True
        for testid in source["tests"]:
            if testid not in old_results:
                flag = False
                break
        if flag:
            unused_sources.add(source["path"])

    try:
        for source in sources:
            if source["path"] in unused_sources:
                print(colorize(f"Skipping {source['path']} as it is unused.", "yellow"))
                continue
            results.update(run_source(source, profile, dry_run, high_priority))
            for k in source["tests"]:
                if k in results:
                    v = results[k]
                    if v["type"] == "simple":
                        results[k] = process_simple_test(k, v)
                else:
                    if k in old_results:
                        results[k] = old_results[k]
                    else:
                        print(colorize(f"Warning: Test {k} defined but not run.", "red"))
    except KeyboardInterrupt:
        print(colorize("Interrupted by user.", "red"))

    sorted_results = {k: results[k] for k in sorted(results.keys())}

    json.dump(
        {"profile": profile, "environment": collect_environment(), "results": sorted_results},
        open(output_file, "w", encoding="utf-8"),
        separators=(",", ":"),
    )


def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    profiles = json.load(open("profiles.json", "r", encoding="utf-8"))

    parser = argparse.ArgumentParser(description="Run algorithm benchmarks.")
    parser.add_argument("--profile", type=str, help="Profile to use for the benchmark", required=False)
    parser.add_argument("--all-profiles", action="store_true", help="Run all profiles from profiles.json", required=False)
    parser.add_argument(
        "-s",
        "--source",
        type=str,
        help="Single config file or directory containing source files",
        required=False,
        default="benchmarks",
    )
    parser.add_argument("--device", type=str, help="Device name to use in results", required=False, default=None)
    parser.add_argument(
        "-o", "--output", type=str, help="File to save output, or result directory if all-profiles is enabled", required=False
    )
    parser.add_argument(
        "--results-index", type=str, help="Path to results index file", required=False, default="results_index.json"
    )
    parser.add_argument("--rerun", "-f", action="store_true", help="Do not skip existing tests", required=False, default=False)
    parser.add_argument("--dry-run", action="store_true", help="Doesn't actually run tests", required=False, default=False)
    parser.add_argument(
        "-p",
        "--high-priority",
        action="store_true",
        help="Run benchmarks with high priority (may require admin/root)",
        required=False,
        default=False,
    )
    parser.add_argument("--test-filter", type=str, help="Run only tests matching this regex", required=False, default=None)
    args = parser.parse_args()

    if args.all_profiles:
        if not args.output:
            args.output = "results/"
        if not args.device:
            args.device = input("Enter device name: ").strip()

        if os.path.exists(args.output) is False:
            os.makedirs(args.output)

        if os.path.exists("results_index.json"):
            results_index = json.load(open("results_index.json", "r"))
        else:
            results_index = []

        for profile_name, profile in profiles.items():
            output_file = os.path.join(args.output, f"{args.device}_{profile_name.replace(' ', '_')}.json")
            print()
            run(
                profile,
                "benchmarks",
                output_file,
                rerun=args.rerun,
                dry_run=args.dry_run,
                high_priority=args.high_priority,
                test_filter=args.test_filter,
            )
            results_index = [entry for entry in results_index if entry["path"] != output_file]
            results_index.append({"name": f"{args.device} {profile_name}", "path": output_file})
            results_index.sort(key=lambda x: x["name"])
            json.dump(results_index, open(args.results_index, "w"))
    else:
        if not args.output:
            args.output = "results.json"
        if args.profile:
            if args.profile not in profiles:
                print(colorize(f"Profile '{args.profile}' not found in profiles.json.", "red"))
                for k, v in profiles.items():
                    print(colorize(f"Available profile: {k} - {v['name']}", "yellow"))
                exit(1)
            profile = profiles[args.profile]
        else:
            profile = next(iter(profiles.values()))
        run(
            profile,
            args.source,
            args.output,
            rerun=args.rerun,
            dry_run=args.dry_run,
            high_priority=args.high_priority,
            test_filter=args.test_filter,
        )


if __name__ == "__main__":
    main()
