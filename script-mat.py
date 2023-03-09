import os
import time

FILE = "mat-bueno.csv"
TMP_FILE = "tmp.txt"
COMPILED_FILE = f"./mult_mat_cua"

if(not os.path.isfile(COMPILED_FILE)):
    os.system(f"./compilar {COMPILED_FILE}")

os.system(f"rm -f {FILE}")
os.system(f"echo  >> {FILE}")

SIZES = [128, 256, 512, 1024]
THREADS = [1, 2, 4, 8, 16]
FILAS = [1, 2, 4, 8]

os.system(f"echo \"Size;Threads;Rows;Time\" >> {FILE}")
for i in SIZES:
    for t in THREADS:
        for f in FILAS:
            os.system(f"{COMPILED_FILE} {i} {f} {t} >> {FILE}")