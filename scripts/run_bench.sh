#!/usr/bin/env bash
set -euo pipefail

# Archivo de salida
CSV="./data/results_v2.csv"
mkdir -p data plots
: > "$CSV"   # truncamos el CSV

# Modos a probar
MODES=("lower" "upper")

# 10 niveles de % alfabético: 0..100
ALPHAS=(0 10 20 30 40 50 60 70 80 90 100)

# 2 alineamientos: 0 (desalineado) y 1 (alineado)
ALIGNS=(0 1)

# Repeticiones para mediana
REPEAT=9

# Generamos ~50+ tamaños (mezcla log y lineal)
SIZES=()
# Log-spaced desde 64 hasta 8MiB aprox.
val=64
while [ $val -le $((8*1024*1024)) ]; do
  SIZES+=($val)
  # multiplicamos por ~1.25 para ir “log”
  val=$(python3 - <<PY
v=$val
print(int(max(v*1.25, v+32)))
PY
)
done
# Asegurar 50+ tamaños (si faltan, agregamos lineales)
while [ ${#SIZES[@]} -lt 50 ]; do
  last=${SIZES[-1]}
  SIZES+=($((last + last/10 + 32)))
done

echo "Iniciando barrido..."
for mode in "${MODES[@]}"; do
  for alpha in "${ALPHAS[@]}"; do
    for align in "${ALIGNS[@]}"; do
      for size in "${SIZES[@]}"; do
        ./bench --mode=$mode --size=$size --alpha=$alpha --aligned=$align --repeat=$REPEAT --csv="$CSV"
      done
    done
  done
done

echo "Listo. CSV en $CSV"
