#ifndef ENCODING_H
#define ENCODING_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

i32 nst_check_utf8_bytes(u8 *byte, usize len);
i32 nst_cp1252_to_utf8(i8 *str, i8 byte);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ENCODING_H