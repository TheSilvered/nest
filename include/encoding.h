#ifndef ENCODING_H
#define ENCODING_H

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef enum _Nst_CPID
{
    NST_CP_ASCII,
    NST_CP_UTF8,
    NST_CP_UTF16,
    NST_CP_UTF32,
    NST_CP_1252
}
Nst_CPID;

EXPORT typedef i32 (*Nst_CheckBytesFunc)(void *str, usize len);
EXPORT typedef u32 (*Nst_ToUTF32Func)(void *str);
EXPORT typedef i32 (*Nst_FromUTF32Func)(u32 ch, void *buf);

EXPORT typedef struct _Nst_CP
{
    usize ch_size;
    usize mult_max_sz;
    usize mult_min_sz;
    const i8 *name;
    Nst_CheckBytesFunc check_bytes;
    Nst_ToUTF32Func to_utf32;
    Nst_FromUTF32Func from_utf32;
}
Nst_CP;

extern Nst_CP nst_cp_utf8;
extern Nst_CP nst_cp_utf16;
extern Nst_CP nst_cp_utf32;
extern Nst_CP nst_cp_1252;

EXPORT i32 nst_check_ascii_bytes(u8 *str, usize len);
EXPORT u32 nst_ascii_to_utf32(u8 *str);
EXPORT i32 nst_ascii_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_utf8_bytes(u8 *str, usize len);
EXPORT u32 nst_utf8_to_utf32(u8 *str);
EXPORT i32 nst_utf8_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_utf16_bytes(u16 *str, usize len);
EXPORT u32 nst_utf16_to_utf32(u16 *str);
EXPORT i32 nst_utf16_from_utf32(u32 ch, u16 *str);

EXPORT i32 nst_check_utf32_bytes(u32 *str, usize len);
EXPORT u32 nst_utf32_to_utf32(u32 *str);
EXPORT i32 nst_utf32_from_utf32(u32 ch, u32 *str);

EXPORT i32 nst_check_1252_bytes(u8 *str, usize len);
EXPORT u32 nst_1252_to_utf32(u8 *str);
EXPORT i32 nst_1252_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_cp1252_to_utf8(i8 *str, i8 byte);
EXPORT i32 nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len);

EXPORT bool nst_translate_cp(Nst_CP *from,
                             Nst_CP *to,
                             void   *from_buf,
                             usize   from_len,
                             void  **to_buf,
                             usize  *to_len,
                             Nst_OpErr *err);

EXPORT Nst_CP *nst_cp(Nst_CPID cpid);

EXPORT wchar_t *nst_char_to_wchar_t(i8 *str, usize len, Nst_OpErr *err);
EXPORT i8 *nst_wchar_t_to_char(wchar_t *str, usize len, Nst_OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ENCODING_H
