#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <vector>

// Búfer grande estático para "ensuciar" caché entre mediciones.
static std::vector<std::uint8_t> g_big(64 * 1024 * 1024, 1); // 64 MiB

void flush_caches_like() {
    // Leemos de forma espaciada para recorrer líneas de cache
    volatile std::uint64_t sum = 0;
    for (std::size_t i = 0; i < g_big.size(); i += 64) {
        sum += g_big[i];
    }
    (void)sum; // evitar que el compilador lo elimine
}

double gib_per_s(std::size_t bytes, double ns) {
    if (ns <= 0.0) return 0.0;
    double s = ns * 1e-9;
    return (bytes / (1024.0 * 1024.0 * 1024.0)) / s;
}
