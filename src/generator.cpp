#include "generator.h"
#include <random>

/*
  Implementación simple del generador.
  owner mantiene la memoria; devolvemos un puntero que cumple la alineación.
*/

uint8_t* alloc_buffer(std::vector<uint8_t>& owner, size_t nbytes, size_t align, bool aligned) {
    owner.resize(nbytes + align);
    uintptr_t base = reinterpret_cast<uintptr_t>(owner.data());
    uintptr_t aligned_ptr = (base + (align - 1)) & ~(uintptr_t)(align - 1);
    if (!aligned) aligned_ptr += 1; // forzar desalineado
    return reinterpret_cast<uint8_t*>(aligned_ptr);
}

void fill_bytes_alpha(uint8_t* p, size_t n, int alpha_pct, uint64_t seed) {
    if (alpha_pct < 0) alpha_pct = 0;
    if (alpha_pct > 100) alpha_pct = 100;

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> prob(0, 99);
    std::uniform_int_distribution<int> pickAlpha(0, 51);   // 52 letras
    std::uniform_int_distribution<int> pickOther(32, 126); // ASCII imprimible

    for (size_t i = 0; i < n; ++i) {
        bool choose_alpha = prob(rng) < alpha_pct;
        if (choose_alpha) {
            int v = pickAlpha(rng);
            p[i] = (v < 26) ? (uint8_t)('A' + v) : (uint8_t)('a' + (v - 26));
        } else {
            int c;
            do { c = pickOther(rng); } while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
            p[i] = (uint8_t)c;
        }
    }
}
