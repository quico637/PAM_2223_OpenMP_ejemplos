import os
import time

FILE = "23.csv"
COMPILED_FILE = f"./mult_mat_cua_bloq"

print(f"Compiling {COMPILED_FILE}...")
os.system(f"./compilar {COMPILED_FILE}")

os.system(f"rm -f {FILE}")
os.system(f"echo  >> {FILE}")

DIM_MAT = [512, 1024, 2048]
TAM_BLO = [4, 16, 32]
THREADS = [1, 2, 4, 8]

os.system(f"echo \"Threads;Size;Block Size;Time\" >> {FILE}")

for t in THREADS:
    for i in DIM_MAT:
        for b in TAM_BLO:
            os.system(f"{COMPILED_FILE} {i} {b} {t} >> {FILE}")