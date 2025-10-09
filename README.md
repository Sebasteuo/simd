 # Taller SIMD 

Este proyecto compara una versión **serial** y una versión **SIMD (AVX2)** para convertir texto entre mayúsculas y minúsculas. 
Incluye un generador de datos, un programa de medición y scripts para barridos y gráficas.


> - En **x86_64 con AVX2** usamos intrinsics reales (cargas/guardas sin requerir alineación).
> - En **ARM/aarch64** (por ejemplo, Mac/Parallels) la ruta “avx2” usa un **fallback escalar** para que compile. 
>   El speedup real se va a ver en una máquina x86_64 con AVX2.

---

## 1) ¿Qué hay aquí? (mapa rápido)

```
taller-simd/
├─ README.md                  
├─ Makefile                   ← build con g++
├─ src/
│  ├─ generator.h/.cpp        ← generador de buffers: tamaño, alineación y % de letras
│  ├─ case_converter_serial.cpp← conversión serial in-place
│  ├─ case_converter_SIMD.cpp  ← conversión SIMD (AVX2; fallback en ARM)
│  ├─ utils.h/.cpp            ← timer, “flush de caché” y utilidades
│  └─ bench.cpp               ← programa de pruebas y medición (CSV)
├─ scripts/
│  ├─ run_bench.sh            ← barrido grande (≥50 tamaños × 10 % × 2 alineamientos)
│  └─ plot.py                 ← genera gráficos a partir del CSV
├─ data/                      ← CSVs de resultados (se crea al correr)
└─ plots/                     ← imágenes PNG de gráficos (se crea al correr)
```

---

## 2) Requisitos e instalación rápida (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y g++ make python3 python3-matplotlib
```

---

## 3) Compilar

```bash
make clean && make -j
```
Se genera el binario `./bench`.

---

## 4) Prueba rápida (valida y mide)

Esto corre **serial** y **avx2** (en ARM, “avx2” es fallback) y escribe resultados a CSV:

```bash
./bench --mode=lower --size=65536 --alpha=70 --aligned=1 --repeat=5 --csv=./data/results_v2.csv
```

Debería verse algo como:
```
OK | mode=lower | N=65536 | alpha=70% | aligned=1 | serial=... ns | avx2=... ns | CSV=./data/results_v2.csv
```

El programa:
- Genera un buffer con ~70% de letras (alineado a 32B).
- Ejecuta **serial** y **avx2** sobre copias iguales del buffer.
- **Valida** que ambas salidas sean idénticas (byte a byte).
- Repite varias veces, toma la **mediana** y agrega **2 filas** al CSV (serial y avx2).

---

## 5) Barrido completo + gráficas

Ejecuta el experimento grande: **≥50 tamaños × 10 niveles de % letras (0..100) × 2 alineamientos (0,1)** para `lower` y `upper`.

```bash
bash scripts/run_bench.sh
python3 scripts/plot.py ./data/results_v2.csv ./plots/
```

Esto crea:
- `plots/speedup_vs_size.png`
- `plots/throughput_vs_alpha.png`

Para abrir (en Linux con entorno gráfico):
```bash
xdg-open plots/speedup_vs_size.png
xdg-open plots/throughput_vs_alpha.png
```

---

## 6) Cómo usar `bench` (parámetros)

- `--mode=lower|upper` : conversión deseada.
- `--size=N`           : tamaño del buffer en bytes.
- `--alpha=0..100`     : % aproximado de letras (A–Z, a–z).
- `--aligned=0|1`      : 1 = puntero alineado a 32B, 0 = desalineado (+1 byte).
- `--repeat=R`         : repeticiones para la **mediana** (recomendado ≥5).
- `--csv=RUTA`         : ruta del CSV de salida (por defecto `./data/results_v2.csv`).

Ejemplos:
```bash
# Alineado, 256 KiB, 70% letras, lower
./bench --mode=lower --size=262144 --alpha=70 --aligned=1 --repeat=9 --csv=./data/results_v2.csv

# Desalineado, 1 MiB, 40% letras, upper
./bench --mode=upper --size=1048576 --alpha=40 --aligned=0 --repeat=9 --csv=./data/results_v2.csv
```

**Formato del CSV (sin encabezado):**
```
mode,size,alpha_pct,aligned,algo,ns_per_call,bytes_per_call,GiB_s
```
- `algo` ∈ {`serial`, `avx2`}.
- `GiB_s` = throughput calculado con `bytes_per_call` y `ns_per_call`.

---

## 7) Randall...

1) Verificar AVX2 y ambiente:
```bash
grep -i 'avx\|avx2' /proc/cpuinfo
g++ --version
uname -a
```
2) Compilar:
```bash
make clean && make -j
```
3) Correr el barrido oficial:
```bash
bash scripts/run_bench.sh
```
4) Generar gráficas:
```bash
python3 scripts/plot.py ./data/results_v2.csv ./plots/
```
5) Enviar resultados:
- `data/results_v2.csv`
- `plots/speedup_vs_size.png`
- `plots/throughput_vs_alpha.png`

> En x86_64 con AVX2, deberías ver **speedup > 1×** (avx2 más rápido que serial), sobre todo en tamaños medianos/grandes.  
> En ARM/VM, “avx2” es fallback escalar y el speedup tiende a ~1×.

---

## 8) Problemas comunes (y soluciones rápidas)

- **No compila `immintrin.h`**: estás en ARM/aarch64. Es normal. El código usa guardas y cae en fallback. El AVX2 real se ve en x86_64.
- **Permiso denegado en `run_bench.sh`**: `chmod +x scripts/run_bench.sh`.
- **CSV muy chico**: corré una prueba básica primero y revisá que no haya errores de ejecución.
- **No salen las gráficas**: instalá `matplotlib` y verificá que `results_v2.csv` tenga muchas filas (`wc -l data/results_v2.csv`).

---

Notas técnicas rápidas

- El generador controla **tamaño**, **alineación** (32B o desalineado +1) y **% de letras**.  
- La versión serial cambia el byte solo si es letra (`±0x20`).  
- La versión AVX2 procesa bloques de 32B con máscaras por rango y maneja la cola con código escalar.  
- Las mediciones usan **mediana** y hacemos un recorrido de un buffer grande entre repeticiones para reducir efectos de caché.

