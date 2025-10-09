#include <cstddef>
#include <cstdint>

/*
  Convierte en el mismo arreglo (in-place).
  - Si to_lower = true: A..Z -> a..z (suma 0x20)
  - Si to_lower = false: a..z -> A..Z (resta 0x20)
  - El resto de caracteres se deja igual.
*/
extern "C" void case_convert_serial(uint8_t* data, size_t n, bool to_lower) {
    for (size_t i = 0; i < n; ++i) {
        uint8_t c = data[i];
        if (to_lower) {
            if (c >= 'A' && c <= 'Z') data[i] = c + 0x20;
        } else {
            if (c >= 'a' && c <= 'z') data[i] = c - 0x20;
        }
    }
}
