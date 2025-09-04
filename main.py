import math
import os
import argparse
import json
import sys
import subprocess
import shutil
import random
import hashlib


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
    ns = [2**i for i in range(64)]

    if not power_of_two:
        ns += [10**i for i in range(1, 10)]
        ns += [2 * 10**i for i in range(1, 10)]
        ns += [5 * 10**i for i in range(1, 10)]

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


def compile_source(source, profile, defs={}):
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

    compile_command = profile["build_command"].format(
        output=output_path, source_path=source_path, defines=" ".join(f"-D{key}={value}" for key, value in defs.items())
    )
    print(colorize(compile_command, "yellow"))

    os.system(compile_command)
    if not os.path.exists(output_path):
        print(colorize(f"Compilation failed with command: {compile_command}", "red"))
        exit(1)

    return output_path


def handle_simple_test(ret, testid, line, input_data):
    global tests
    test = tests[testid]
    values = line.split(":")[1].strip().split(" ")
    cur = {}
    for i, value in enumerate(values):
        cur[test["template"][i]] = float(value)
    ret[testid]["data"].append(cur)


def process_simple_test(test):
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
        remove_outliers = max(1, len(values) // 10)
        values = values[remove_outliers:-remove_outliers]

        mean = sum(values) / len(values)
        stddev = math.sqrt(sum((x - mean) ** 2 for x in values) / len(values))

        mean_c = mean / complexity_fn(n)
        stddev_c = stddev / complexity_fn(n)

        stats.append(
            {
                "n": n,
                "mean": mean,
                "stddev": stddev,
                "mean_c": mean_c,
                "stddev_c": stddev_c,
                "min": min(values),
                "max": max(values),
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


def run_source(source, profile):
    global tests

    ret = {}
    inputs = []
    if source["input"]["type"] == "generator":
        inputs = generator(**source["input"]["params"])
    else:
        raise ValueError(f"Unsupported input type: {source['input']['type']}")

    for input_data in inputs:
        output_path = compile_source(source, profile, input_data.get("defs", {}))
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
                if testid in tests:
                    test = tests[testid]
                    if testid not in ret:
                        ret[testid] = test.copy()
                        ret[testid]["data"] = []
                    if test["type"] == "simple":
                        handle_simple_test(ret, testid, line, input_data)
    return ret


def collect_environment():
    env = {
        "platform": sys.platform,
        "python_version": sys.version,
    }
    try:
        # Collect g++ version info
        gpp_proc = subprocess.run(["g++", "-v"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        env["g++"] = gpp_proc.stderr if gpp_proc.stderr else gpp_proc.stdout
    except Exception as e:
        env["g++"] = f"Error: {e}"

    try:
        # Collect lscpu info
        lscpu_proc = subprocess.run(["lscpu"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        env["cpuinfo"] = lscpu_proc.stdout if lscpu_proc.stdout else lscpu_proc.stderr
    except Exception as e:
        env["cpuinfo"] = f"Error: {e}"

    return env


def hash_obj(obj):
    return hashlib.md5(json.dumps(obj, sort_keys=True).encode()).hexdigest()


def run(profile, source_path, output_file, rerun=False):
    print(colorize(f"Using profile: {profile['name']}", "green"))
    random.seed(42)

    global tests
    global results
    global old_results

    tests = {}
    results = {}
    sources = []
    if os.path.isfile(source_path):
        cfg = json.load(open(source_path, "r"))
        if "tests" in cfg:
            tests.update(cfg["tests"])
        if "sources" in cfg:
            for source in cfg["sources"]:
                source["path"] = os.path.join(os.path.dirname(source_path), source["path"])
                sources.append(source)
    else:
        for dirpath, dirnames, filenames in os.walk(source_path):
            for file in filenames:
                if file.endswith(".json"):
                    cfg = json.load(open(os.path.join(dirpath, file), "r"))

                    source_hash = []
                    source_files = []
                    for source in cfg["sources"]:
                        source_config_hash = hash_obj(source)
                        source["tests"] = list(cfg["tests"].keys())
                        path = source["path"] = os.path.join(dirpath, source["path"])
                        source_hash.append(
                            [hashlib.md5(open(path, "rb").read().replace(b"\r", b"")).hexdigest(), source_config_hash]
                        )
                        source_files.append(path)
                        sources.append(source)

                    source_hash = hash_obj(source_hash)

                    for testid, test in cfg["tests"].items():
                        test["source_files"] = source_files
                        test["source_hash"] = source_hash
                        test["test_hash"] = hash_obj(test)
                        tests[testid] = test

    print(colorize(f"Found {len(sources)} source files and {len(tests)} tests.", "green"))

    old_results = {}
    if (not rerun) and os.path.exists(output_file):
        old_results_file = json.load(open(output_file, "r"))
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

    for source in sources:
        if source["path"] in unused_sources:
            print(colorize(f"Skipping {source['path']} as it is unused.", "yellow"))
            continue
        try:
            results.update(run_source(source, profile))
        except KeyboardInterrupt:
            print(colorize("Interrupted by user.", "red"))
            break

    for k in sorted(tests.keys()):
        if k in results:
            v = results[k]
            if v["type"] == "simple":
                results[k] = process_simple_test(v)
        else:
            if k in old_results:
                results[k] = old_results[k]
            else:
                print(colorize(f"Warning: Test {k} defined but not run.", "red"))

    sorted_results = {k: results[k] for k in sorted(results.keys())}

    json.dump(
        {"profile": profile, "environment": collect_environment(), "results": sorted_results},
        open(output_file, "w"),
        separators=(",", ":"),
    )


def main():
    profiles = json.load(open("profiles.json", "r"))

    parser = argparse.ArgumentParser(description="Run C++ benchmarks.")
    parser.add_argument("--profile", type=str, help="Profile to use for the benchmark", required=False)
    parser.add_argument(
        "--source",
        type=str,
        help="Single config file or directory containing source files",
        required=False,
        default="benchmarks",
    )
    parser.add_argument("--output", type=str, help="File to save output", required=False, default="results.json")
    parser.add_argument("--rerun", action="store_true", help="Do not skip existing tests", required=False, default=False)
    args = parser.parse_args()

    if args.profile:
        if args.profile not in profiles:
            print(colorize(f"Profile '{args.profile}' not found in profiles.json.", "red"))
            for k, v in profiles.items():
                print(colorize(f"Available profile: {k} - {v['name']}", "yellow"))
            exit(1)
        profile = profiles[args.profile]
    else:
        profile = next(iter(profiles.values()))

    run(profile, args.source, args.output, args.rerun)


if __name__ == "__main__":
    main()
