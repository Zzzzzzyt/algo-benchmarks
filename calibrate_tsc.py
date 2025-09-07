import sys
import subprocess
import os


def calibrate_tsc():
    if sys.platform == "linux":
        raw = subprocess.run("dmesg | grep -i tsc", shell=True, check=True, capture_output=True).stdout
        for line in raw.decode().split("\n"):
            line = line.lower().strip()
            if "tsc" in line and "mhz" in line:
                print(line)
                parts = line.split()
                for i, part in enumerate(parts):
                    if "mhz" in part:
                        try:
                            mhz = float(parts[i - 1])
                            if mhz > 1000 and mhz < 10000:
                                print(f"Using TSC frequency from dmesg: {mhz} MHz")
                                with open("tsc_freq.txt", "w") as f:
                                    f.write(f"{mhz/1000:.10f}\n")
                                return
                        except ValueError:
                            continue

    if sys.platform == "win32":
        subprocess.run(["g++", "calibrate_tsc.cpp", "-O2", "-o", "calibrate_tsc.exe"], check=True)
        subprocess.run(["calibrate_tsc.exe"], check=True)
    elif sys.platform == "linux":
        subprocess.run(["g++", "calibrate_tsc.cpp", "-O2", "-o", "calibrate_tsc"], check=True)
        subprocess.run(["calibrate_tsc"], check=True)

    if os.path.exists("tsc_freq.txt"):
        mhz = float(open("tsc_freq.txt").read().strip())
        print(f"TSC Frequency: {mhz*1000} MHz")
    else:
        print("Failed to determine TSC frequency.")


if __name__ == "__main__":
    calibrate_tsc()
