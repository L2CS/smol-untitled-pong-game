#!/usr/bin/env python3

import glob
import subprocess


def clang_format():
    """Run clang-format recursively and fix up any formatting"""

    print("Running clang_format ...")
    files = []
    h_files = glob.glob("src/**/*.h", recursive=True)
    cpp_files = glob.glob("src/**/*.cpp", recursive=True)
    files = h_files + cpp_files

    for f in files:
        print(f"clang-format file: {f}")
        subprocess.run(f"clang-format -i --style=file {f}", shell=True)


if __name__ == "__main__":
    clang_format()
