#ifndef ENCODING_H
#define ENCODING_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT i32 nst_check_utf8_bytes (u8 *byte, usize len);
EXPORT i32 nst_cp1252_to_utf8(i8 *str, i8 byte);
EXPORT i32 nst_check_utf16_bytes(u16 *byte, usize len);
EXPORT i32 nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ENCODING_H