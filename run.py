import math
import os
import argparse
import json
import sys
import subprocess
import shutil


def get_complexity_fn(complexity):
    if complexity == "O(n)":
        return lambda n: n
    elif complexity == "O(n^2)":
        return lambda n: n**2
    elif complexity == "O(log n)":
        return lambda n: math.log2(n)
    else:
        raise ValueError(f"Unknown complexity: {complexity}")


def generator(type="exp", lower_bound=1, upper_bound=20, complexity="O(n)", max_repeats=1_000_000, estimated_constant=1):
    lower_bound = 2**lower_bound
    upper_bound = 2**upper_bound

    if type == "exp":
        ns = (
            [2**i for i in range(64)]
            + [10**i for i in range(1, 10)]
            + [2 * 10**i for i in range(1, 10)]
            + [5 * 10**i for i in range(1, 10)]
        )
    else:
        raise ValueError(f"Unknown generator type: {type}")

    ns.sort()
    ret = []
    complexity_fn = get_complexity_fn(complexity)
    for n in ns:
        if n < lower_bound or n > upper_bound:
            continue
        micro_repeats = min(max(1, math.ceil(50_000_000 / complexity_fn(n) / estimated_constant)), max_repeats)
        ret.append({"defs": {"BENCHMARK_N": n, "BENCHMARK_MICRO_REPEATS": micro_repeats}})
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
    print(compile_command)

    os.system(compile_command)
    if not os.path.exists(output_path):
        print(f"Compilation failed with command: {compile_command}")
        exit(1)

    return output_path


def handle_simple_test(testid, line, input_data):
    global tests
    test = tests[testid]
    values = line.split(":")[1].strip().split(" ")
    cur = {}
    for i, value in enumerate(values):
        cur[test["template"][i]] = float(value)
    results[testid]["data"].append(cur)


def process_simple_test(testid):
    test = results[testid]
    stat = {}
    for sample in test["data"]:
        n = sample["n"]
        if n not in stat:
            stat[n] = []
        stat[n].append(sample["time_ns"] / sample["micro_repeats"])

    complexity_fn = get_complexity_fn(test["complexity"])

    # Compute statistics for each key
    stats = []
    for n, values in stat.items():
        values.sort()
        values = values[1:-1]

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


def run_source(source, profile):
    global tests
    global results

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
            print(p.stdout.decode())
            if p.returncode != 0:
                print(f"Error running {source['path']} with input {input_data}:")
                print(p.stderr.decode())
                exit(1)
            for line in p.stdout.decode().splitlines():
                testid = line.split(":")[0].strip()
                if testid in tests:
                    test = tests[testid]
                    if testid not in results:
                        results[testid] = test.copy()
                        results[testid]["data"] = []
                    if test["type"] == "simple":
                        handle_simple_test(testid, line, input_data)


def main():
    global tests
    global results

    sources = []
    tests = {}
    results = {}
    profiles = json.load(open("profiles.json", "r"))

    parser = argparse.ArgumentParser(description="Run C++ benchmarks.")
    parser.add_argument("--profile", type=str, help="Profile to use for the benchmark", required=False)
    parser.add_argument(
        "--source-dir", type=str, help="Directory containing source files", required=False, default="benchmarks"
    )
    parser.add_argument("--output", type=str, help="File to save output", required=False, default="results.json")
    args = parser.parse_args()

    if args.profile:
        if args.profile not in profiles:
            print(f"Profile '{args.profile}' not found in profiles.json.")
            for k, v in profiles.items():
                print(f"Available profile: {k} - {v['name']}")
            exit(1)
        profile = profiles[args.profile]
    else:
        profile = next(iter(profiles.values()))

    print(f"Using profile: {profile['name']}")

    for dirpath, dirnames, filenames in os.walk(args.source_dir):
        for file in filenames:
            if file.endswith(".json"):
                cfg = json.load(open(os.path.join(dirpath, file), "r"))
                if "tests" in cfg:
                    tests.update(cfg["tests"])
                if "sources" in cfg:
                    for source in cfg["sources"]:
                        source["path"] = os.path.join(dirpath, source["path"])
                        sources.append(source)

    print(f"Found {len(sources)} source files and {len(tests)} tests.")

    for source in sources:
        run_source(source, profile)

    for k, v in results.items():
        if v["type"] == "simple":
            process_simple_test(k)

    json.dump(results, open(args.output, "w"), indent=4)


if __name__ == "__main__":
    main()
