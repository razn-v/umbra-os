#include <stddef.h>
#include <stdint.h>

namespace Format {

size_t int_to_str(char* buffer, int64_t integer);

size_t uint_to_hex(char* buffer, uint64_t integer);

}
