#ifndef ENCODING_H
#define ENCODING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

int nst_check_utf8_bytes(unsigned char *byte, size_t len);
int nst_cp1252_to_utf8(char *str, char byte);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ENCODING_H