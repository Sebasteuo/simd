#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

/*
  Funciones para crear y llenar un buffer de bytes.
  - alloc_buffer: devuelve un puntero con la alineación pedida (p.ej., 32 bytes).
                  Si aligned=false, lo desalineamos a propósito (+1 byte).
  - fill_bytes_alpha: llena con letras A-Z/a-z según un porcentaje, el resto son
                      caracteres imprimibles no alfabéticos.
*/

uint8_t* alloc_buffer(std::vector<uint8_t>& owner, size_t nbytes, size_t align, bool aligned);
void fill_bytes_alpha(uint8_t* p, size_t n, int alpha_pct, uint64_t seed = 0xC0FFEEULL);
