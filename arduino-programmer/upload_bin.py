#!/usr/bin/env python3
import argparse
import serial
import subprocess
import time
from pathlib import Path


def main():
    ap = argparse.ArgumentParser(description="Upload program .bin to Arduino programmer")
    ap.add_argument("--port", required=True, help="Serial port (e.g. /dev/ttyACM0 or COM5)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--file", required=True, help="Program binary file")
    ap.add_argument("--asm", help="Assembly source to compile before upload")
    ap.add_argument("--assembler", default="compiler/assembler", help="Path to assembler binary")
    ap.add_argument("--addr", type=int, default=0, help="Address to select after upload (0-31)")
    args = ap.parse_args()

    if args.asm:
        asm_path = Path(args.asm)
        if not asm_path.exists():
            raise SystemExit(f"ASM not found: {asm_path}")
        asm_out = Path(args.file)
        asm_out.parent.mkdir(parents=True, exist_ok=True)
        result = subprocess.run(
            [args.assembler, str(asm_path), str(asm_out)],
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            print(result.stdout)
            print(result.stderr)
            raise SystemExit("Assembler failed")

    data = Path(args.file).read_bytes()
    if len(data) > 32:
        # Address bus in the current sketch is 5-bit (0..31)
        raise SystemExit("Program too large (max 32 bytes with current address bus)")

    if args.addr < 0 or args.addr > 31:
        raise SystemExit("addr out of range (0-31)")

    with serial.Serial(args.port, args.baud, timeout=2) as ser:
        time.sleep(2)
        # Wait for board to reboot and print ready line (if any)
        start = time.time()
        ready_seen = False
        while time.time() - start < 3:
            line = ser.readline()
            if not line:
                continue
            text = line.decode(errors="ignore").strip()
            if "Ready" in text:
                ready_seen = True
                break

        ser.reset_input_buffer()

        # Upload
        ser.write(b'U')
        ser.write(bytes([len(data) & 0xFF, (len(data) >> 8) & 0xFF]))
        ser.write(data)
        ser.flush()

        resp = ser.read_until(b"\n")
        print(resp.decode(errors="ignore").strip() or "<no response>")

        # Select address after upload
        ser.write(b'A')
        ser.write(str(args.addr).encode("ascii") + b"\n")
        ser.flush()


if __name__ == "__main__":
    main()
