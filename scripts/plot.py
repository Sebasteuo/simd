import sys, os, csv, math
import matplotlib.pyplot as plt

# Uso: python3 scripts/plot.py data/results_v2.csv plots/
if len(sys.argv) < 3:
    print("Uso: python3 scripts/plot.py <csv> <outdir>")
    sys.exit(1)

csv_path = sys.argv[1]
out_dir = sys.argv[2]
os.makedirs(out_dir, exist_ok=True)

# Leemos CSV sin encabezado: 
# mode,size,alpha_pct,aligned,algo,ns_per_call,bytes_per_call,GiB_s
rows = []
with open(csv_path, newline='') as f:
    r = csv.reader(f)
    for line in r:
        if not line or len(line) < 8: 
            continue
        mode = line[0]
        size = int(line[1])
        alpha = int(line[2])
        aligned = int(line[3])
        algo = line[4]
        ns = float(line[5])
        bytes_call = int(line[6])
        gib_s = float(line[7])
        rows.append((mode,size,alpha,aligned,algo,ns,bytes_call,gib_s))

# Helper: filtrar
def filt(**kw):
    res = []
    for t in rows:
        ok = True
        if 'mode' in kw and t[0] != kw['mode']: ok=False
        if 'aligned' in kw and t[3] != kw['aligned']: ok=False
        if 'alpha' in kw and t[2] != kw['alpha']: ok=False
        res.append(t) if ok else None
    return res

# 1) Speedup vs tamaño (ejemplo: mode=lower, alpha=70, aligned=1)
sel = filt(mode='lower', aligned=1, alpha=70)
# armamos diccionario por (size -> {algo: ns})
by_size = {}
for mode,size,alpha,aligned,algo,ns,bytes_call,gib_s in sel:
    by_size.setdefault(size, {})[algo] = ns

xs = sorted(by_size.keys())
speedups = []
for sz in xs:
    d = by_size[sz]
    if 'serial' in d and 'avx2' in d and d['avx2'] > 0:
        speedups.append(d['serial'] / d['avx2'])
    else:
        speedups.append(float('nan'))

plt.figure()
plt.plot(xs, speedups)
plt.xscale('log')
plt.xlabel('Tamaño (bytes)')
plt.ylabel('Speedup (serial/avx2)')
plt.title('Speedup vs tamaño (mode=lower, alpha=70%, aligned=1)')
plt.grid(True, which='both')
plt.tight_layout()
plt.savefig(os.path.join(out_dir, 'speedup_vs_size.png'))

# 2) Throughput vs % alfabético (≈1MiB, aligned=1, mode=lower)
# buscamos tamaños cercanos a 1 MiB
target = 1*1024*1024
tol_low, tol_high = int(target*0.8), int(target*1.2)
sel2 = [t for t in rows if t[0]=='lower' and t[3]==1 and tol_low<=t[1]<=tol_high]
# agrupamos por (alpha, algo) tomando la mediana de GiB/s
from statistics import median
by_alpha_algo = {}
for mode,size,alpha,aligned,algo,ns,bytes_call,gib_s in sel2:
    by_alpha_algo.setdefault((alpha,algo), []).append(gib_s)

alphas_sorted = sorted(set(a for (a,algo) in by_alpha_algo.keys()))
algos = sorted(set(algo for (a,algo) in by_alpha_algo.keys()))

plt.figure()
for algo in algos:
    ys = []
    for a in alphas_sorted:
        vals = by_alpha_algo.get((a,algo), [])
        if vals:
            ys.append(median(vals))
        else:
            ys.append(float('nan'))
    plt.plot(alphas_sorted, ys, label=algo)
plt.xlabel('% alfabético')
plt.ylabel('GiB/s (mediana)')
plt.title('Throughput vs % alfabético (~1MiB, aligned=1, mode=lower)')
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(out_dir, 'throughput_vs_alpha.png'))

print("Gráficas guardadas en:", out_dir)
