#include <cstdio>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include "generator.h"
#include "utils.h"

// Firmas
extern "C" void case_convert_serial(uint8_t* data, size_t n, bool to_lower);
extern "C" void case_convert_avx2(uint8_t* data, size_t n, bool to_lower);

/*
  Mide SERIAL y AVX2 (o fallback) con:
  - repetición y mediana
  - "limpieza de caché" entre repeticiones
  - salida a CSV con columnas completas (incluye GiB/s)

  Flags:
    --mode=lower|upper
    --size=N
    --alpha=0..100
    --aligned=0|1
    --repeat=R
    --csv=RUTA   (por defecto ./data/results_v2.csv)
*/

struct Args {
    const char* mode = "lower";
    std::size_t size = 256 * 1024;
    int alpha = 70;
    bool aligned = true;
    int repeat = 7;
    const char* csv = "./data/results_v2.csv";
};

static bool starts_with(const char* s, const char* pref) {
    return std::strncmp(s, pref, std::strlen(pref)) == 0;
}

static Args parse_args(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        if (starts_with(argv[i], "--mode="))      a.mode   = argv[i] + 7;
        else if (starts_with(argv[i], "--size=")) a.size   = std::strtoull(argv[i] + 7, nullptr, 10);
        else if (starts_with(argv[i], "--alpha="))a.alpha  = std::atoi(argv[i] + 8);
        else if (starts_with(argv[i], "--aligned=")) a.aligned = std::atoi(argv[i] + 10)!=0;
        else if (starts_with(argv[i], "--repeat="))  a.repeat  = std::atoi(argv[i] + 9);
        else if (starts_with(argv[i], "--csv="))     a.csv     = argv[i] + 6;
        else { std::fprintf(stderr, "Flag no reconocido: %s\n", argv[i]); std::exit(1); }
    }
    if (a.alpha < 0) a.alpha = 0;
    if (a.alpha > 100) a.alpha = 100;
    if (a.repeat < 1) a.repeat = 1;
    return a;
}

// CSV extendido: mode,size,alpha_pct,aligned,algo,ns_per_call,bytes_per_call,GiB_s
static void append_csv(const char* path, const char* mode, std::size_t size, int alpha_pct,
                       int aligned, const char* algo, std::uint64_t ns) {
    std::FILE* f = std::fopen(path, "a");
    if (!f) { std::perror("csv"); std::exit(2); }
    double gib = gib_per_s(size, (double)ns);
    std::fprintf(f, "%s,%zu,%d,%d,%s,%lu,%zu,%.6f\n",
                 mode, size, alpha_pct, aligned, algo, (unsigned long)ns, size, gib);
    std::fclose(f);
}

static bool buffers_equal(const std::vector<std::uint8_t>& a,
                          const std::vector<std::uint8_t>& b, std::size_t& idx) {
    if (a.size() != b.size()) { idx = (std::size_t)-1; return false; }
    for (std::size_t i = 0; i < a.size(); ++i) if (a[i] != b[i]) { idx = i; return false; }
    return true;
}

static std::uint64_t median_ns(std::vector<std::uint64_t>& v) {
    std::nth_element(v.begin(), v.begin()+v.size()/2, v.end());
    return v[v.size()/2];
}

int main(int argc, char** argv) {
    Args a = parse_args(argc, argv);
    bool to_lower = (std::strcmp(a.mode, "lower") == 0);

    system("mkdir -p data"); // salida
    const std::size_t align = 32;

    // Buffer base
    std::vector<std::uint8_t> owner;
    std::uint8_t* base = alloc_buffer(owner, a.size, align, a.aligned);
    fill_bytes_alpha(base, a.size, a.alpha, 12345);

    // Validación de correctitud (una pasada serial vs avx2/fallback)
    {
        std::vector<std::uint8_t> s(base, base + a.size);
        std::vector<std::uint8_t> v(base, base + a.size);
        case_convert_serial(s.data(), s.size(), to_lower);
        case_convert_avx2 (v.data(), v.size(), to_lower);
        std::size_t idx=0;
        if (!buffers_equal(s, v, idx)) {
            std::fprintf(stderr, "Error: serial vs avx2 difieren en %zu\n", idx);
            return 3;
        }
    }

    // Medición serial (mediana) con flush entre repeticiones
    std::vector<std::uint64_t> times; times.reserve(a.repeat);
    for (int r=0; r<a.repeat; ++r) {
        std::vector<std::uint8_t> tmp(base, base + a.size);
        flush_caches_like();
        Timer t; t.start();
        case_convert_serial(tmp.data(), tmp.size(), to_lower);
        times.push_back(t.ns());
    }
    std::uint64_t med_serial = median_ns(times);

    // Medición avx2/fallback (mediana) con flush
    times.clear();
    for (int r=0; r<a.repeat; ++r) {
        std::vector<std::uint8_t> tmp(base, base + a.size);
        flush_caches_like();
        Timer t; t.start();
        case_convert_avx2(tmp.data(), tmp.size(), to_lower);
        times.push_back(t.ns());
    }
    std::uint64_t med_avx2 = median_ns(times);

    append_csv(a.csv, a.mode, a.size, a.alpha, a.aligned ? 1 : 0, "serial", med_serial);
    append_csv(a.csv, a.mode, a.size, a.alpha, a.aligned ? 1 : 0, "avx2",  med_avx2);

    std::printf("OK | mode=%s | N=%zu | alpha=%d%% | aligned=%d | serial=%lu ns | avx2=%lu ns | CSV=%s\n",
                a.mode, a.size, a.alpha, a.aligned ? 1 : 0,
                (unsigned long)med_serial, (unsigned long)med_avx2, a.csv);
    return 0;
}
