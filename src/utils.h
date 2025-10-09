#pragma once
#include <chrono>
#include <cstdint>
#include <vector>

/*
  Utilidades simples:
  - Timer: medir en nanosegundos.
  - flush_caches_like(): toca un búfer grande para "ensuciar" cachés entre corridas.
  - gib_per_s(): throughput a partir de bytes y ns.
*/

struct Timer {
    using clock = std::chrono::steady_clock;
    clock::time_point t0;
    void start() { t0 = clock::now(); }
    uint64_t ns() const {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            clock::now() - t0
        ).count();
    }
};

// Recorre un búfer grande para reducir efectos de caché entre repeticiones.
void flush_caches_like();

// Calcula GiB/s dados bytes procesados y tiempo en ns.
double gib_per_s(std::size_t bytes, double ns);
