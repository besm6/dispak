#!/usr/bin/env python3
import subprocess
import sys
import os
import tempfile


def main():
    if len(sys.argv) != 4:
        print("Usage: stdin-eof.py <dispak> <disk-dir> <input-file>")
        return 2

    app, disk_dir, input_file = sys.argv[1:]
    with tempfile.TemporaryDirectory(prefix="dispak-stdin-eof-") as home:
        env = os.environ.copy()
        env["HOME"] = home
        try:
            result = subprocess.run(
                [app, f"--path={disk_dir}", input_file],
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=env,
                timeout=2,
            )
        except subprocess.TimeoutExpired:
            print("FATAL: dispak did not terminate after stdin EOF")
            return 1

    if result.returncode != 0:
        sys.stdout.buffer.write(result.stdout)
        sys.stderr.buffer.write(result.stderr)
        print(f"FATAL: dispak failed with status {result.returncode}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
