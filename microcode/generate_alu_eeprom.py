#!/usr/bin/env python3
from pathlib import Path

# 4-bit input (0..15) -> 6-bit output: M S3 S2 S1 S0 Cn (bit5..bit0)
# Active-high. Unused codes map to 0.
#
# Sequential mapping:
# 0 AND
# 1 OR
# 2 XOR
# 3 NOT A
# 4 INC (A+1)
# 5 DEC (A-1)
# 6 ADD (A+B)
# 7 SUB (A-B)
# 8 ALUONES (0xFF)
# 9 ALUZEROS (0x00)

OPS = {
    0: dict(M=1, S=0b1011, Cn=0),  # AND (Cn don't care)
    1: dict(M=1, S=0b1110, Cn=0),  # OR
    2: dict(M=1, S=0b0110, Cn=0),  # XOR
    3: dict(M=1, S=0b0000, Cn=0),  # NOT A
    4: dict(M=0, S=0b0000, Cn=0),  # INC A
    5: dict(M=0, S=0b1111, Cn=1),  # DEC A
    6: dict(M=0, S=0b1001, Cn=1),  # ADD A+B
    7: dict(M=0, S=0b0110, Cn=0),  # SUB A-B
    8: dict(M=1, S=0b1100, Cn=0),  # ALUONES
    9: dict(M=1, S=0b0011, Cn=0),  # ALUZEROS
}

def pack(M, S, Cn):
    return ((M & 1) << 5) | ((S & 0xF) << 1) | (Cn & 1)

def main():
    data = []
    for code in range(16):
        if code in OPS:
            op = OPS[code]
            b = pack(op["M"], op["S"], op["Cn"])
        else:
            b = 0
        data.append(b)

    out = Path("alu_eeprom.bin")
    out.write_bytes(bytes(data))
    print(f"Wrote {out} ({len(data)} bytes)")

if __name__ == "__main__":
    main()
