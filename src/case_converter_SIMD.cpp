#include <cstddef>
#include <cstdint>

/*
  Implementación condicional:
  - En x86_64 con AVX2: usamos intrinsics AVX2 (carga/almacenamiento desalineado).
  - En otras arquitecturas (p.ej. ARM/aarch64): usamos un fallback simple
    para que compile y funcione, pero sin aceleración SIMD (tu compañero lo correrá en x86).
*/

#if defined(__x86_64__) || defined(_M_X64)
  #include <immintrin.h>

  static inline __m256i is_between(__m256i x, uint8_t lo, uint8_t hi) {
      __m256i vlo = _mm256_set1_epi8((char)(lo - 1));
      __m256i vhi = _mm256_set1_epi8((char)(hi + 1));
      __m256i gtlo = _mm256_cmpgt_epi8(x, vlo); // x >= lo
      __m256i lthi = _mm256_cmpgt_epi8(vhi, x); // x <= hi
      return _mm256_and_si256(gtlo, lthi);
  }

  extern "C" void case_convert_avx2(uint8_t* data, size_t n, bool to_lower) {
      const __m256i vDelta = _mm256_set1_epi8(0x20);
      size_t i = 0;
      for (; i + 31 < n; i += 32) {
          __m256i v = _mm256_loadu_si256((const __m256i*)(data + i));
          if (to_lower) {
              __m256i mask = is_between(v, 'A', 'Z');
              __m256i add  = _mm256_and_si256(mask, vDelta);
              v = _mm256_add_epi8(v, add);
          } else {
              __m256i mask = is_between(v, 'a', 'z');
              __m256i sub  = _mm256_and_si256(mask, vDelta);
              v = _mm256_sub_epi8(v, sub);
          }
          _mm256_storeu_si256((__m256i*)(data + i), v);
      }
      // Cola escalar
      for (; i < n; ++i) {
          uint8_t c = data[i];
          if (to_lower) {
              if (c >= 'A' && c <= 'Z') data[i] = c + 0x20;
          } else {
              if (c >= 'a' && c <= 'z') data[i] = c - 0x20;
          }
      }
  }

#else
  // Fallback genérico (sin SIMD) para arquitecturas no x86_64 (ej. ARM/aarch64)
  extern "C" void case_convert_avx2(uint8_t* data, size_t n, bool to_lower) {
      for (size_t i = 0; i < n; ++i) {
          uint8_t c = data[i];
          if (to_lower) {
              if (c >= 'A' && c <= 'Z') data[i] = c + 0x20;
          } else {
              if (c >= 'a' && c <= 'z') data[i] = c - 0x20;
          }
      }
  }
#endif
