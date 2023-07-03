#ifndef ENCODING_H
#define ENCODING_H

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef enum _Nst_CPID
{
    NST_CP_UNKNOWN = -1,
    NST_CP_ASCII,
    NST_CP_UTF8,
    NST_CP_UTF16,
    NST_CP_UTF16BE,
    NST_CP_UTF16LE,
    NST_CP_UTF32,
    NST_CP_UTF32BE,
    NST_CP_UTF32LE,
    NST_CP_1250,
    NST_CP_1251,
    NST_CP_1252,
    NST_CP_1253,
    NST_CP_1254,
    NST_CP_1255,
    NST_CP_1256,
    NST_CP_1257,
    NST_CP_1258,
    NST_CP_LATIN1,
    NST_CP_ISO8859_1 = NST_CP_LATIN1
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

extern Nst_CP nst_cp_ascii;
extern Nst_CP nst_cp_utf8;
extern Nst_CP nst_cp_utf16;
extern Nst_CP nst_cp_utf16be;
extern Nst_CP nst_cp_utf16le;
extern Nst_CP nst_cp_utf32;
extern Nst_CP nst_cp_utf32be;
extern Nst_CP nst_cp_utf32le;
extern Nst_CP nst_cp_1250;
extern Nst_CP nst_cp_1251;
extern Nst_CP nst_cp_1252;
extern Nst_CP nst_cp_1253;
extern Nst_CP nst_cp_1254;
extern Nst_CP nst_cp_1255;
extern Nst_CP nst_cp_1256;
extern Nst_CP nst_cp_1257;
extern Nst_CP nst_cp_1258;
extern Nst_CP nst_cp_iso8859_1;

EXPORT i32 nst_check_ascii_bytes(u8 *str, usize len);
EXPORT u32 nst_ascii_to_utf32(u8 *str);
EXPORT i32 nst_ascii_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_utf8_bytes(u8 *str, usize len);
EXPORT u32 nst_utf8_to_utf32(u8 *str);
EXPORT i32 nst_utf8_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_utf16_bytes(u16 *str, usize len);
EXPORT u32 nst_utf16_to_utf32(u16 *str);
EXPORT i32 nst_utf16_from_utf32(u32 ch, u16 *str);

EXPORT i32 nst_check_utf16be_bytes(u8 *str, usize len);
EXPORT u32 nst_utf16be_to_utf32(u8 *str);
EXPORT i32 nst_utf16be_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_utf16le_bytes(u8 *str, usize len);
EXPORT u32 nst_utf16le_to_utf32(u8 *str);
EXPORT i32 nst_utf16le_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_utf32_bytes(u32 *str, usize len);
EXPORT u32 nst_utf32_to_utf32(u32 *str);
EXPORT i32 nst_utf32_from_utf32(u32 ch, u32 *str);

EXPORT i32 nst_check_utf32be_bytes(u8 *str, usize len);
EXPORT u32 nst_utf32be_to_utf32(u8 *str);
EXPORT i32 nst_utf32be_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_utf32le_bytes(u8 *str, usize len);
EXPORT u32 nst_utf32le_to_utf32(u8 *str);
EXPORT i32 nst_utf32le_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1250_bytes(u8 *str, usize len);
EXPORT u32 nst_1250_to_utf32(u8 *str);
EXPORT i32 nst_1250_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1251_bytes(u8 *str, usize len);
EXPORT u32 nst_1251_to_utf32(u8 *str);
EXPORT i32 nst_1251_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1252_bytes(u8 *str, usize len);
EXPORT u32 nst_1252_to_utf32(u8 *str);
EXPORT i32 nst_1252_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1253_bytes(u8 *str, usize len);
EXPORT u32 nst_1253_to_utf32(u8 *str);
EXPORT i32 nst_1253_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1254_bytes(u8 *str, usize len);
EXPORT u32 nst_1254_to_utf32(u8 *str);
EXPORT i32 nst_1254_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1255_bytes(u8 *str, usize len);
EXPORT u32 nst_1255_to_utf32(u8 *str);
EXPORT i32 nst_1255_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1256_bytes(u8 *str, usize len);
EXPORT u32 nst_1256_to_utf32(u8 *str);
EXPORT i32 nst_1256_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1257_bytes(u8 *str, usize len);
EXPORT u32 nst_1257_to_utf32(u8 *str);
EXPORT i32 nst_1257_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_1258_bytes(u8 *str, usize len);
EXPORT u32 nst_1258_to_utf32(u8 *str);
EXPORT i32 nst_1258_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_check_iso8859_1_bytes(u8 *str, usize len);
EXPORT u32 nst_iso8859_1_to_utf32(u8 *str);
EXPORT i32 nst_iso8859_1_from_utf32(u32 ch, u8 *str);

EXPORT i32 nst_cp1252_to_utf8(i8 *str, i8 byte);
EXPORT i32 nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len);

EXPORT bool nst_translate_cp(Nst_CP *from,
                             Nst_CP *to,
                             void   *from_buf,
                             usize   from_len,
                             void  **to_buf,
                             usize  *to_len);

EXPORT isize nst_check_string_cp(Nst_CP *cp, void *str, usize str_len);

EXPORT Nst_CP *nst_cp(Nst_CPID cpid);

#ifdef WINDOWS
EXPORT Nst_CPID nst_acp();
#endif

EXPORT wchar_t *nst_char_to_wchar_t(i8 *str, usize len);
EXPORT i8 *nst_wchar_t_to_char(wchar_t *str, usize len);

EXPORT bool nst_is_valid_cp(u32 cp);

EXPORT Nst_CPID nst_check_bom(i8 *str, usize len, i32 *bom_size);
EXPORT Nst_CPID nst_detect_encoding(i8 *str, usize len, i32 *bom_size);
EXPORT Nst_CPID nst_encoding_from_name(i8 *name);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ENCODING_H
