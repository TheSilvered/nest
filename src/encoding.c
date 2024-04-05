#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include "encoding.h"
#include "mem.h"
#include "lib_import.h"
#include "format.h"

#ifdef Nst_WIN
#include <windows.h>
#endif // !Nst_WIN

Nst_CP Nst_cp_ascii = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "ASCII",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_ascii_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_ascii_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_ascii_from_utf32,
};

Nst_CP Nst_cp_utf8 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8),
    .name = "UTF-8",
    .bom = "\xef\xbb\xbf",
    .bom_size = 3,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_utf8_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_utf8_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_utf8_from_utf32,
};

Nst_CP Nst_cp_ext_utf8 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8),
    .name = "extUTF-8",
    .bom = "\xef\xbb\xbf",
    .bom_size = 3,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_ext_utf8_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_ext_utf8_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_ext_utf8_from_utf32,
};

Nst_CP Nst_cp_utf16 = {
    .ch_size = sizeof(u16),
    .mult_max_sz = sizeof(u16) * 2,
    .mult_min_sz = sizeof(u16),
    .name = "UTF-16",
#if Nst_BYTEORDER == Nst_LITTLE_ENDIAN
    .bom = "\xff\xfe",
#else
    .bom = "\xfe\xff",
#endif
    .bom_size = 2,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_utf16_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_utf16_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_utf16_from_utf32,
};

Nst_CP Nst_cp_utf16be = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8) * 2,
    .name = "UTF-16BE",
    .bom = "\xfe\xff",
    .bom_size = 2,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_utf16be_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_utf16be_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_utf16be_from_utf32,
};

Nst_CP Nst_cp_utf16le = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8) * 2,
    .name = "UTF-16LE",
    .bom = "\xff\xfe",
    .bom_size = 2,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_utf16le_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_utf16le_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_utf16le_from_utf32,
};

Nst_CP Nst_cp_ext_utf16 = {
    .ch_size = sizeof(u16),
    .mult_max_sz = sizeof(u16) * 2,
    .mult_min_sz = sizeof(u16),
    .name = "extUTF-16",
#if Nst_BYTEORDER == Nst_LITTLE_ENDIAN
    .bom = "\xff\xfe",
#else
    .bom = "\xfe\xff",
#endif
    .bom_size = 2,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_ext_utf16_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_ext_utf16_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_ext_utf16_from_utf32,
};

Nst_CP Nst_cp_ext_utf16be = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8) * 2,
    .name = "extUTF-16BE",
    .bom = "\xfe\xff",
    .bom_size = 2,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_ext_utf16be_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_ext_utf16be_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_ext_utf16be_from_utf32,
};

Nst_CP Nst_cp_ext_utf16le = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8) * 2,
    .name = "extUTF-16LE",
    .bom = "\xff\xfe",
    .bom_size = 2,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_ext_utf16le_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_ext_utf16le_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_ext_utf16le_from_utf32,
};

Nst_CP Nst_cp_utf32 = {
    .ch_size = sizeof(u32),
    .mult_max_sz = sizeof(u32),
    .mult_min_sz = sizeof(u32),
    .name = "extUTF-32",
#if Nst_BYTEORDER == Nst_LITTLE_ENDIAN
    .bom = "\xff\xfe\x00\x00",
#else
    .bom = "\x00\x00\xfe\xff",
#endif
    .bom_size = 4,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_utf32_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_utf32_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_utf32_from_utf32,
};

Nst_CP Nst_cp_utf32be = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8) * 4,
    .name = "UTF-32BE",
    .bom = "\x00\x00\xfe\xff",
    .bom_size = 4,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_utf32be_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_utf32be_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_utf32be_from_utf32,
};

Nst_CP Nst_cp_utf32le = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8) * 4,
    .name = "UTF-32LE",
    .bom = "\xff\xfe\x00\x00",
    .bom_size = 4,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_utf32le_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_utf32le_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_utf32le_from_utf32,
};

Nst_CP Nst_cp_1250 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1250",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1250_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1250_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1250_from_utf32,
};

Nst_CP Nst_cp_1251 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1251",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1251_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1251_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1251_from_utf32,
};

Nst_CP Nst_cp_1252 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1252",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1252_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1252_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1252_from_utf32,
};

Nst_CP Nst_cp_1253 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1253",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1253_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1253_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1253_from_utf32,
};

Nst_CP Nst_cp_1254 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1254",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1254_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1254_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1254_from_utf32,
};

Nst_CP Nst_cp_1255 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1255",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1255_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1255_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1255_from_utf32,
};

Nst_CP Nst_cp_1256 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1256",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1256_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1256_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1256_from_utf32,
};

Nst_CP Nst_cp_1257 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1257",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1257_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1257_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1257_from_utf32,
};

Nst_CP Nst_cp_1258 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1258",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1258_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1258_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1258_from_utf32,
};

Nst_CP Nst_cp_iso8859_1 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "ISO-8859-1",
    .bom = NULL,
    .bom_size = 0,
    .check_bytes = (Nst_CheckBytesFunc)Nst_check_1258_bytes,
    .to_utf32    = (Nst_ToUTF32Func)Nst_1258_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)Nst_1258_from_utf32,
};

i32 cp_1250_map[128] = {
    0x20ac,     -1, 0x201a,     -1, 0x201e, 0x2026, 0x2020, 0x2021,
        -1, 0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179,
        -1, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
        -1, 0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a,
    0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b,
    0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c,
    0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,
    0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
    0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,
    0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
    0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,
    0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
    0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,
    0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9
};

i32 cp_1251_map[128] = {
    0x0402, 0x0403, 0x201a, 0x0453, 0x201e, 0x2026, 0x2020, 0x2021,
    0x20ac,     -1, 0x0409, 0x2039, 0x040a, 0x040c, 0x040b, 0x040f,
    0x0452, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x0098, 0x2122, 0x0459, 0x203a, 0x045a, 0x045c, 0x045b, 0x045f,
    0x00a0, 0x040e, 0x045e, 0x0408, 0x00a4, 0x0490, 0x00a6, 0x00a7,
    0x0401, 0x00a9, 0x0404, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x0407,
    0x00b0, 0x00b1, 0x0406, 0x0456, 0x0491, 0x00b5, 0x00b6, 0x00b7,
    0x0451, 0x2116, 0x0454, 0x00bb, 0x0458, 0x0405, 0x0455, 0x0457,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f
};

i32 cp_1252_map[128] = {
    0x20ac,     -1, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152,     -1, 0x017d,     -1,
        -1, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153,     -1, 0x017e, 0x0178,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

i32 cp_1253_map[128] = {
    0x20ac,     -1, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
        -1, 0x2030,     -1, 0x2039,     -1,     -1,     -1,     -1,
        -1, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
        -1, 0x2122,     -1, 0x203a,     -1,     -1,     -1,     -1,
    0x00a0, 0x0385, 0x0386, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9,     -1, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x2015,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x0384, 0x00b5, 0x00b6, 0x00b7,
    0x0388, 0x0389, 0x038a, 0x00bb, 0x038c, 0x00bd, 0x038e, 0x038f,
    0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
    0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
    0x03a0, 0x03a1,     -1, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
    0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
    0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7,
    0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
    0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7,
    0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce,     -1
};

i32 cp_1254_map[128] = {
    0x20ac,     -1, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152,     -1,     -1,     -1,
        -1, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153,     -1,     -1, 0x0178,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x011e, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0130, 0x015e, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x011f, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0131, 0x015f, 0x00ff
};

i32 cp_1255_map[128] = {
    0x20ac,     -1, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030,     -1, 0x2039,     -1,     -1,     -1,     -1,
        -1, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122,     -1, 0x203a,     -1,     -1,     -1,     -1,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x20aa, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00d7, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00f7, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x05b0, 0x05b1, 0x05b2, 0x05b3, 0x05b4, 0x05b5, 0x05b6, 0x05b7,
    0x05b8, 0x05b9,     -1, 0x05bb, 0x05bc, 0x05bd, 0x05be, 0x05bf,
    0x05c0, 0x05c1, 0x05c2, 0x05c3, 0x05f0, 0x05f1, 0x05f2, 0x05f3,
    0x05f4,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
    0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05d4, 0x05d5, 0x05d6, 0x05d7,
    0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05dd, 0x05de, 0x05df,
    0x05e0, 0x05e1, 0x05e2, 0x05e3, 0x05e4, 0x05e5, 0x05e6, 0x05e7,
    0x05e8, 0x05e9, 0x05ea,     -1,     -1, 0x200e, 0x200f,     -1
};

i32 cp_1256_map[128] = {
    0x20ac, 0x067e, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0679, 0x2039, 0x0152, 0x0686, 0x0698, 0x0688,
    0x06af, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x06a9, 0x2122, 0x0691, 0x203a, 0x0153, 0x200c, 0x200d, 0x06ba,
    0x00a0, 0x060c, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x06be, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x061b, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x061f,
    0x06c1, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627,
    0x0628, 0x0629, 0x062a, 0x062b, 0x062c, 0x062d, 0x062e, 0x062f,
    0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00d7,
    0x0637, 0x0638, 0x0639, 0x063a, 0x0640, 0x0641, 0x0642, 0x0643,
    0x00e0, 0x0644, 0x00e2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x0649, 0x064a, 0x00ee, 0x00ef,
    0x064b, 0x064c, 0x064d, 0x064e, 0x00f4, 0x064f, 0x0650, 0x00f7,
    0x0651, 0x00f9, 0x0652, 0x00fb, 0x00fc, 0x200e, 0x200f, 0x06d2
};

i32 cp_1257_map[128] = {
    0x20ac,     -1, 0x201a,     -1, 0x201e, 0x2026, 0x2020, 0x2021,
        -1, 0x2030,     -1, 0x2039,     -1, 0x00a8, 0x02c7, 0x00b8,
        -1, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
        -1, 0x2122,     -1, 0x203a,     -1, 0x00af, 0x02db,     -1,
    0x00a0,     -1, 0x00a2, 0x00a3, 0x00a4,     -1, 0x00a6, 0x00a7,
    0x00d8, 0x00a9, 0x0156, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00c6,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00f8, 0x00b9, 0x0157, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00e6,
    0x0104, 0x012e, 0x0100, 0x0106, 0x00c4, 0x00c5, 0x0118, 0x0112,
    0x010c, 0x00c9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012a, 0x013b,
    0x0160, 0x0143, 0x0145, 0x00d3, 0x014c, 0x00d5, 0x00d6, 0x00d7,
    0x0172, 0x0141, 0x015a, 0x016a, 0x00dc, 0x017b, 0x017d, 0x00df,
    0x0105, 0x012f, 0x0101, 0x0107, 0x00e4, 0x00e5, 0x0119, 0x0113,
    0x010d, 0x00e9, 0x017a, 0x0117, 0x0123, 0x0137, 0x012b, 0x013c,
    0x0161, 0x0144, 0x0146, 0x00f3, 0x014d, 0x00f5, 0x00f6, 0x00f7,
    0x0173, 0x0142, 0x015b, 0x016b, 0x00fc, 0x017c, 0x017e, 0x02d9
};

i32 cp_1258_map[128] = {
    0x20ac,     -1, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030,     -1, 0x2039, 0x0152,     -1,     -1,     -1,
        -1, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122,     -1, 0x203a, 0x0153,     -1,     -1, 0x0178,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x0300, 0x00cd, 0x00ce, 0x00cf,
    0x0110, 0x00d1, 0x0309, 0x00d3, 0x00d4, 0x01a0, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x01af, 0x0303, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x0301, 0x00ed, 0x00ee, 0x00ef,
    0x0111, 0x00f1, 0x0323, 0x00f3, 0x00f4, 0x01a1, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x01b0, 0x20ab, 0x00ff
};

i32 Nst_check_ascii_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return *str <= 0x7f ? 1 : -1;
}

u32 Nst_ascii_to_utf32(u8 *str)
{
    return (u32)*str;
}

i32 Nst_ascii_from_utf32(u32 ch, u8 *str)
{
    if (ch > 0x7f)
        return -1;
    *str = (u8)ch;
    return 1;
}

static u32 utf8_cp(u8 c, u8 *str, i32 len)
{
    switch (len) {
    case 1:
        return (u32)c;
    case 2:
        return ((c & 0x1f) << 6) | (*str & 0x3f);
#if Nst_BYTEORDER == Nst_BIG_ENDIAN
    case 3: {
        u16 n = *(u16 *)str;
        return ((c & 0xf) << 12) | ((n & 0x3f00) >> 2) | (n & 0x3f);
    } default: {
        u32 n = *(u32 *)(str - 1);
        return ((c & 0x7) << 18)
             | ((n & 0x3f0000) >> 4)
             | ((n & 0x3f00) >> 2)
             | (n & 0x3f);
    }
#else
    case 3: {
        u16 n = *(u16 *)str;
        return ((c & 0xf) << 12) | ((n & 0x3f) << 6) | ((n & 0x3f00) >> 8);
    } default: {
        u32 n = *(u32 *)(str - 1);
        return ((c & 0x7) << 18)
            | ((n & 0x3f00) << 4)
            | ((n & 0x3f0000) >> 10)
            | ((n & 0x3f000000) >> 24);
    }
#endif
    }
}

i32 Nst_check_utf8_bytes(u8 *str, usize len)
{
    i32 ch_len = Nst_check_ext_utf8_bytes(str, len);
    if (ch_len == -1)
        return -1;
    u8 ch1 = *str++;
    u32 ch = utf8_cp(ch1, str, ch_len);
    if (ch >= 0xd800 && ch <= 0xdfff)
        return -1;
    return ch_len;
}

u32 Nst_utf8_to_utf32(u8 *str)
{
    return Nst_ext_utf8_to_utf32(str);
}

i32 Nst_utf8_from_utf32(u32 ch, u8 *str)
{
    if (ch >= 0xd800 && ch <= 0xdfff)
        return -1;

    return Nst_ext_utf8_from_utf32(ch, str);
}

i32 Nst_check_ext_utf8_bytes(u8 *str, usize len)
{
    u8 ch1 = *str++;

    if ((ch1 & 0x80) == 0)
        return 1;
    else if ((ch1 & 0xe0) == 0xc0) {
        if (len < 2 || (*str & 0xc0) != 0x80)
            return -1;
        return 2;
    } else if ((ch1 & 0xf0) == 0xe0) {
        if (len < 3 || (*(u16 *)str & 0xc0c0) != 0x8080)
            return -1;
        return 3;
    } else if ((ch1 & 0xf8) == 0xf0) {
#if Nst_BYTEORDER == Nst_BIG_ENDIAN
        if (len < 4 || (*(u32 *)(str - 1) & 0xc0c0c0) != 0x808080)
#else
        if (len < 4 || (*(u32 *)(str - 1) & 0xc0c0c000) != 0x80808000)
#endif
            return -1;
        return 4;
    } else
        return -1;
}

u32 Nst_ext_utf8_to_utf32(u8 *str)
{
    i32 len;
    u8 ch1 = *str++;

    if ((ch1 & 0x80) == 0)
        len = 1;
    else if ((ch1 & 0b11100000) == 0b11000000)
        len = 2;
    else if ((ch1 & 0b11110000) == 0b11100000)
        len = 3;
    else
        len = 4;

    return utf8_cp(ch1, str, len);
}

i32 Nst_ext_utf8_from_utf32(u32 ch, u8 *str)
{
    if (ch <= 0x7f) {
        *str = (i8)ch;
        return 1;
    } else if (ch <= 0x7ff) {
        *str++ = 0b11000000 | (i8)(ch >> 6);
        *str   = 0b10000000 | (i8)(ch & 0x3f);
        return 2;
    } else if (ch <= 0xffff) {
        *str++ = 0b11100000 | (i8)(ch >> 12);
        *str++ = 0b10000000 | (i8)(ch >> 6 & 0x3f);
        *str   = 0b10000000 | (i8)(ch & 0x3f);
        return 3;
    } else if (ch > 0x10ffff)
        return -1;

    *str++ = 0b11110000 | (i8)(ch >> 18);
    *str++ = 0b10000000 | (i8)(ch >> 12 & 0x3f);
    *str++ = 0b10000000 | (i8)(ch >> 6 & 0x3f);
    *str   = 0b10000000 | (i8)(ch & 0x3f);
    return 4;
}

i32 Nst_check_utf16_bytes(u16 *str, usize len)
{
    u16 ch = *str;
    if (ch < 0xd800 || ch > 0xdfff)
        return 1;
    if (ch >= 0xdc00 || len < 2)
        return -1;
    ch = *++str;
    if (ch < 0xdc00 || ch > 0xdfff)
        return -1;
    return 2;
}

u32 Nst_utf16_to_utf32(u16 *str)
{
    if (*str < 0xd800 || *str > 0xdfff)
        return (u32)*str;
    return ((str[0] & 0x3ff) << 10) + (str[1] & 0x3ff) + 0x10000;
}

i32 Nst_utf16_from_utf32(u32 ch, u16 *str)
{
    if (ch > 0x10ffff || (ch >= 0xd800 && ch <= 0xdfff))
        return -1;
    if (ch < 0xffff) {
        *str = (u16)ch;
        return 1;
    }
    ch -= 0x10000;
    *str++ = 0xd800 + ((ch >> 10) & 0x3ff);
    *str   = 0xdc00 + (ch & 0x3ff);
    return 2;
}

i32 Nst_check_utf16be_bytes(u8 *str, usize len)
{
    if (len < 2)
        return -1;

    if (len >= 4) {
        u16 wstr[2] = {
            ((u16)str[0] << 8) | (u16)str[1],
            ((u16)str[2] << 8) | (u16)str[3]
        };
        return Nst_check_utf16_bytes(wstr, 2) * 2;
    }
    u16 wch = ((u16)str[0] << 8) | (u16)str[1];
    return Nst_check_utf16_bytes(&wch, 1) * 2;
}

u32 Nst_utf16be_to_utf32(u8 *str)
{
    u16 ch1 = ((u16)str[0] << 8) | (u16)str[1];
    if (ch1 < 0xd800 || ch1 > 0xdfff)
        return (u32)ch1;
    u16 ch2 = ((u16)str[2] << 8) | (u16)str[3];
    return ((ch1 & 0x3ff) << 10) + (ch2 & 0x3ff) + 0x10000;
}

i32 Nst_utf16be_from_utf32(u32 ch, u8 *str)
{
    if (ch > 0x10FFFF || (ch >= 0xd800 && ch <= 0xdfff))
        return -1;
    if (ch < 0xffff) {
        str[0] = (u8)(ch >> 8);
        str[1] = (u8)(ch & 0xff);
        return 2;
    }
    ch -= 0x10000;
    u16 ch1 = 0xd800 + ((ch >> 10) & 0x3ff);
    u16 ch2 = 0xdc00 + (ch & 0x3ff);
    str[0] = (u8)(ch1 >> 8);
    str[1] = (u8)(ch1 & 0xff);
    str[2] = (u8)(ch2 >> 8);
    str[3] = (u8)(ch2 & 0xff);
    return 4;
}

i32 Nst_check_utf16le_bytes(u8 *str, usize len)
{
    if (len < 2)
        return -1;

    if (len >= 4) {
        u16 wstr[2] = {
            ((u16)str[1] << 8) | (u16)str[0],
            ((u16)str[3] << 8) | (u16)str[2]
        };
        return Nst_check_utf16_bytes(wstr, 2) * 2;
    }
    u16 wch = ((u16)str[1] << 8) | (u16)str[0];
    return Nst_check_utf16_bytes(&wch, 1) * 2;
}

u32 Nst_utf16le_to_utf32(u8 *str)
{
    u16 ch1 = ((u16)str[1] << 8) | (u16)str[0];
    if (ch1 < 0xd800 || ch1 > 0xdfff)
        return (u32)ch1;
    u16 ch2 = ((u16)str[3] << 8) | (u16)str[2];
    return ((ch1 & 0x3ff) << 10) + (ch2 & 0x3ff) + 0x10000;
}

i32 Nst_utf16le_from_utf32(u32 ch, u8 *str)
{
    if (ch > 0x10FFFF || (ch >= 0xd800 && ch <= 0xdfff))
        return -1;

    if (ch < 0xffff) {
        str[1] = (u8)(ch >> 8);
        str[0] = (u8)(ch & 0xff);
        return 2;
    }

    ch -= 0x10000;
    u16 ch1 = 0xd800 + ((ch >> 10) & 0x3ff);
    u16 ch2 = 0xdc00 + (ch & 0x3ff);
    str[1] = (u8)(ch1 >> 8);
    str[0] = (u8)(ch1 & 0xff);
    str[3] = (u8)(ch2 >> 8);
    str[2] = (u8)(ch2 & 0xff);
    return 4;
}

i32 Nst_check_ext_utf16_bytes(u16 *str, usize len)
{
    u16 ch = *str;
    if (ch < 0xd800 || ch > 0xdfff)
        return 1;
    if (ch >= 0xdc00)
        return 1;
    if (len < 2)
        return -1;

    ch = *++str;
    if (ch < 0xdc00 || ch > 0xdfff)
        return 1;
    return 2;
}

u32 Nst_ext_utf16_to_utf32(u16 *str)
{
    if (*str < 0xd800 || *str > 0xdc00)
        return (u32)*str;
    if (str[1] < 0xdc00 || str[1] > 0xdfff)
        return (u32)*str;
    return ((str[0] & 0x3ff) << 10) + (str[1] & 0x3ff) + 0x10000;
}

i32 Nst_ext_utf16_from_utf32(u32 ch, u16 *str)
{
    if (ch > 0x10ffff)
        return -1;
    if (ch < 0xffff) {
        *str = (u16)ch;
        return 1;
    }
    ch -= 0x10000;
    *str++ = 0xd800 + ((ch >> 10) & 0x3ff);
    *str   = 0xdc00 + (ch & 0x3ff);
    return 2;
}

i32 Nst_check_ext_utf16be_bytes(u8 *str, usize len)
{
    if (len < 2)
        return -1;

    if (len >= 4) {
        u16 wstr[2] = {
            ((u16)str[0] << 8) | (u16)str[1],
            ((u16)str[2] << 8) | (u16)str[3]
        };
        return Nst_check_ext_utf16_bytes(wstr, 2) * 2;
    }
    u16 wch = ((u16)str[0] << 8) | (u16)str[1];
    return Nst_check_ext_utf16_bytes(&wch, 1) * 2;
}

u32 Nst_ext_utf16be_to_utf32(u8 *str)
{
    u16 ch1 = ((u16)str[0] << 8) | (u16)str[1];
    if (ch1 < 0xd800 || ch1 > 0xdc00)
        return (u32)ch1;
    u16 ch2 = ((u16)str[2] << 8) | (u16)str[3];
    if (ch2 < 0xdc00 || ch2 > 0xdfff)
        return (u32)ch1;
    return ((ch1 & 0x3ff) << 10) + (ch2 & 0x3ff) + 0x10000;
}

i32 Nst_ext_utf16be_from_utf32(u32 ch, u8 *str)
{
    if (ch > 0x10FFFF)
        return -1;
    if (ch < 0xffff) {
        str[0] = (u8)(ch >> 8);
        str[1] = (u8)(ch & 0xff);
        return 2;
    }
    ch -= 0x10000;
    u16 ch1 = 0xd800 + ((ch >> 10) & 0x3ff);
    u16 ch2 = 0xdc00 + (ch & 0x3ff);
    str[0] = (u8)(ch1 >> 8);
    str[1] = (u8)(ch1 & 0xff);
    str[2] = (u8)(ch2 >> 8);
    str[3] = (u8)(ch2 & 0xff);
    return 4;
}

i32 Nst_check_ext_utf16le_bytes(u8 *str, usize len)
{
    if (len < 2)
        return -1;

    if (len >= 4) {
        u16 wstr[2] = {
            ((u16)str[1] << 8) | (u16)str[0],
            ((u16)str[3] << 8) | (u16)str[2]
        };
        return Nst_check_ext_utf16_bytes(wstr, 2) * 2;
    }
    u16 wch = ((u16)str[1] << 8) | (u16)str[0];
    return Nst_check_ext_utf16_bytes(&wch, 1) * 2;
}

u32 Nst_ext_utf16le_to_utf32(u8 *str)
{
    u16 ch1 = ((u16)str[1] << 8) | (u16)str[0];
    if (ch1 < 0xd800 || ch1 > 0xdc00)
        return (u32)ch1;
    u16 ch2 = ((u16)str[3] << 8) | (u16)str[2];
    if (ch2 < 0xdc00 || ch2 > 0xdfff)
        return (u32)ch1;
    return ((ch1 & 0x3ff) << 10) + (ch2 & 0x3ff) + 0x10000;
}

i32 Nst_ext_utf16le_from_utf32(u32 ch, u8 *str)
{
    if (ch > 0x10FFFF)
        return -1;

    if (ch < 0xffff) {
        str[1] = (u8)(ch >> 8);
        str[0] = (u8)(ch & 0xff);
        return 2;
    }

    ch -= 0x10000;
    u16 ch1 = 0xd800 + ((ch >> 10) & 0x3ff);
    u16 ch2 = 0xdc00 + (ch & 0x3ff);
    str[1] = (u8)(ch1 >> 8);
    str[0] = (u8)(ch1 & 0xff);
    str[3] = (u8)(ch2 >> 8);
    str[2] = (u8)(ch2 & 0xff);
    return 4;
}

i32 Nst_check_utf32_bytes(u32 *str, usize len)
{
    Nst_UNUSED(str);
    Nst_UNUSED(len);
    return 1;
}

u32 Nst_utf32_to_utf32(u32 *str)
{
    return *str;
}

i32 Nst_utf32_from_utf32(u32 ch, u32 *str)
{
    *str = ch;
    return 1;
}

i32 Nst_check_utf32be_bytes(u8 *str, usize len)
{
    Nst_UNUSED(str);
    if (len < 4)
        return -1;
    return 4;
}

u32 Nst_utf32be_to_utf32(u8 *str)
{
    return (str[0] << 12) + (str[1] << 8) + (str[2] << 4) + str[3];
}

i32 Nst_utf32be_from_utf32(u32 ch, u8 *str)
{
    str[0] = (u8)(ch >> 12);
    str[1] = (u8)((ch >> 8) & 0xff);
    str[2] = (u8)((ch >> 4) & 0xff);
    str[3] = (u8)(ch & 0xff);
    return 4;
}

i32 Nst_check_utf32le_bytes(u8 *str, usize len)
{
    Nst_UNUSED(str);
    if (len < 4)
        return -1;
    return 4;
}

u32 Nst_utf32le_to_utf32(u8 *str)
{
    return (str[3] << 12) + (str[2] << 8) + (str[1] << 4) + str[0];
}

i32 Nst_utf32le_from_utf32(u32 ch, u8 *str)
{
    str[3] = (u8)(ch >> 12);
    str[2] = (u8)((ch >> 8) & 0xff);
    str[1] = (u8)((ch >> 4) & 0xff);
    str[0] = (u8)(ch & 0xff);
    return 4;
}

i32 Nst_check_125x_bytes(u8 *str, i32 *chars)
{
    u8 ch = *str;
    return ch < 0x80 ? 1 : chars[ch - 0x80] != -1 ? 1 : -1;
}

u32 Nst_125x_to_utf32(u8 *str, i32 *chars)
{
    u8 ch = *str;
    return ch < 0x80 ? ch : chars[ch - 0x80];
}

i32 Nst_check_1250_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1250_map);
}

u32 Nst_1250_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1250_map);
}

i32 Nst_1250_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x20ac: *str = 0x80; break; case 0x201a: *str = 0x82; break;
    case 0x201e: *str = 0x84; break; case 0x2026: *str = 0x85; break;
    case 0x2020: *str = 0x86; break; case 0x2021: *str = 0x87; break;
    case 0x2030: *str = 0x89; break; case 0x0160: *str = 0x8a; break;
    case 0x2039: *str = 0x8b; break; case 0x015a: *str = 0x8c; break;
    case 0x0164: *str = 0x8d; break; case 0x017d: *str = 0x8e; break;
    case 0x0179: *str = 0x8f; break; case 0x2018: *str = 0x91; break;
    case 0x2019: *str = 0x92; break; case 0x201c: *str = 0x93; break;
    case 0x201d: *str = 0x94; break; case 0x2022: *str = 0x95; break;
    case 0x2013: *str = 0x96; break; case 0x2014: *str = 0x97; break;
    case 0x2122: *str = 0x99; break; case 0x0161: *str = 0x9a; break;
    case 0x203a: *str = 0x9b; break; case 0x015b: *str = 0x9c; break;
    case 0x0165: *str = 0x9d; break; case 0x017e: *str = 0x9e; break;
    case 0x017a: *str = 0x9f; break; case 0x00a0: *str = 0xa0; break;
    case 0x02c7: *str = 0xa1; break; case 0x02d8: *str = 0xa2; break;
    case 0x0141: *str = 0xa3; break; case 0x00a4: *str = 0xa4; break;
    case 0x0104: *str = 0xa5; break; case 0x00a6: *str = 0xa6; break;
    case 0x00a7: *str = 0xa7; break; case 0x00a8: *str = 0xa8; break;
    case 0x00a9: *str = 0xa9; break; case 0x015e: *str = 0xaa; break;
    case 0x00ab: *str = 0xab; break; case 0x00ac: *str = 0xac; break;
    case 0x00ad: *str = 0xad; break; case 0x00ae: *str = 0xae; break;
    case 0x017b: *str = 0xaf; break; case 0x00b0: *str = 0xb0; break;
    case 0x00b1: *str = 0xb1; break; case 0x02db: *str = 0xb2; break;
    case 0x0142: *str = 0xb3; break; case 0x00b4: *str = 0xb4; break;
    case 0x00b5: *str = 0xb5; break; case 0x00b6: *str = 0xb6; break;
    case 0x00b7: *str = 0xb7; break; case 0x00b8: *str = 0xb8; break;
    case 0x0105: *str = 0xb9; break; case 0x015f: *str = 0xba; break;
    case 0x00bb: *str = 0xbb; break; case 0x013d: *str = 0xbc; break;
    case 0x02dd: *str = 0xbd; break; case 0x013e: *str = 0xbe; break;
    case 0x017c: *str = 0xbf; break; case 0x0154: *str = 0xc0; break;
    case 0x00c1: *str = 0xc1; break; case 0x00c2: *str = 0xc2; break;
    case 0x0102: *str = 0xc3; break; case 0x00c4: *str = 0xc4; break;
    case 0x0139: *str = 0xc5; break; case 0x0106: *str = 0xc6; break;
    case 0x00c7: *str = 0xc7; break; case 0x010c: *str = 0xc8; break;
    case 0x00c9: *str = 0xc9; break; case 0x0118: *str = 0xca; break;
    case 0x00cb: *str = 0xcb; break; case 0x011a: *str = 0xcc; break;
    case 0x00cd: *str = 0xcd; break; case 0x00ce: *str = 0xce; break;
    case 0x010e: *str = 0xcf; break; case 0x0110: *str = 0xd0; break;
    case 0x0143: *str = 0xd1; break; case 0x0147: *str = 0xd2; break;
    case 0x00d3: *str = 0xd3; break; case 0x00d4: *str = 0xd4; break;
    case 0x0150: *str = 0xd5; break; case 0x00d6: *str = 0xd6; break;
    case 0x00d7: *str = 0xd7; break; case 0x0158: *str = 0xd8; break;
    case 0x016e: *str = 0xd9; break; case 0x00da: *str = 0xda; break;
    case 0x0170: *str = 0xdb; break; case 0x00dc: *str = 0xdc; break;
    case 0x00dd: *str = 0xdd; break; case 0x0162: *str = 0xde; break;
    case 0x00df: *str = 0xdf; break; case 0x0155: *str = 0xe0; break;
    case 0x00e1: *str = 0xe1; break; case 0x00e2: *str = 0xe2; break;
    case 0x0103: *str = 0xe3; break; case 0x00e4: *str = 0xe4; break;
    case 0x013a: *str = 0xe5; break; case 0x0107: *str = 0xe6; break;
    case 0x00e7: *str = 0xe7; break; case 0x010d: *str = 0xe8; break;
    case 0x00e9: *str = 0xe9; break; case 0x0119: *str = 0xea; break;
    case 0x00eb: *str = 0xeb; break; case 0x011b: *str = 0xec; break;
    case 0x00ed: *str = 0xed; break; case 0x00ee: *str = 0xee; break;
    case 0x010f: *str = 0xef; break; case 0x0111: *str = 0xf0; break;
    case 0x0144: *str = 0xf1; break; case 0x0148: *str = 0xf2; break;
    case 0x00f3: *str = 0xf3; break; case 0x00f4: *str = 0xf4; break;
    case 0x0151: *str = 0xf5; break; case 0x00f6: *str = 0xf6; break;
    case 0x00f7: *str = 0xf7; break; case 0x0159: *str = 0xf8; break;
    case 0x016f: *str = 0xf9; break; case 0x00fa: *str = 0xfa; break;
    case 0x0171: *str = 0xfb; break; case 0x00fc: *str = 0xfc; break;
    case 0x00fd: *str = 0xfd; break; case 0x0163: *str = 0xfe; break;
    case 0x02d9: *str = 0xff; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_1251_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1251_map);
}

u32 Nst_1251_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1251_map);
}

i32 Nst_1251_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x0402: *str = 0x80; break; case 0x0403: *str = 0x81; break;
    case 0x201a: *str = 0x82; break; case 0x0453: *str = 0x83; break;
    case 0x201e: *str = 0x84; break; case 0x2026: *str = 0x85; break;
    case 0x2020: *str = 0x86; break; case 0x2021: *str = 0x87; break;
    case 0x20ac: *str = 0x88; break; case 0x0409: *str = 0x8a; break;
    case 0x2039: *str = 0x8b; break; case 0x040a: *str = 0x8c; break;
    case 0x040c: *str = 0x8d; break; case 0x040b: *str = 0x8e; break;
    case 0x040f: *str = 0x8f; break; case 0x0452: *str = 0x90; break;
    case 0x2018: *str = 0x91; break; case 0x2019: *str = 0x92; break;
    case 0x201c: *str = 0x93; break; case 0x201d: *str = 0x94; break;
    case 0x2022: *str = 0x95; break; case 0x2013: *str = 0x96; break;
    case 0x2014: *str = 0x97; break; case 0x0098: *str = 0x98; break;
    case 0x2122: *str = 0x99; break; case 0x0459: *str = 0x9a; break;
    case 0x203a: *str = 0x9b; break; case 0x045a: *str = 0x9c; break;
    case 0x045c: *str = 0x9d; break; case 0x045b: *str = 0x9e; break;
    case 0x045f: *str = 0x9f; break; case 0x00a0: *str = 0xa0; break;
    case 0x040e: *str = 0xa1; break; case 0x045e: *str = 0xa2; break;
    case 0x0408: *str = 0xa3; break; case 0x00a4: *str = 0xa4; break;
    case 0x0490: *str = 0xa5; break; case 0x00a6: *str = 0xa6; break;
    case 0x00a7: *str = 0xa7; break; case 0x0401: *str = 0xa8; break;
    case 0x00a9: *str = 0xa9; break; case 0x0404: *str = 0xaa; break;
    case 0x00ab: *str = 0xab; break; case 0x00ac: *str = 0xac; break;
    case 0x00ad: *str = 0xad; break; case 0x00ae: *str = 0xae; break;
    case 0x0407: *str = 0xaf; break; case 0x00b0: *str = 0xb0; break;
    case 0x00b1: *str = 0xb1; break; case 0x0406: *str = 0xb2; break;
    case 0x0456: *str = 0xb3; break; case 0x0491: *str = 0xb4; break;
    case 0x00b5: *str = 0xb5; break; case 0x00b6: *str = 0xb6; break;
    case 0x00b7: *str = 0xb7; break; case 0x0451: *str = 0xb8; break;
    case 0x2116: *str = 0xb9; break; case 0x0454: *str = 0xba; break;
    case 0x00bb: *str = 0xbb; break; case 0x0458: *str = 0xbc; break;
    case 0x0405: *str = 0xbd; break; case 0x0455: *str = 0xbe; break;
    case 0x0457: *str = 0xbf; break; case 0x0410: *str = 0xc0; break;
    case 0x0411: *str = 0xc1; break; case 0x0412: *str = 0xc2; break;
    case 0x0413: *str = 0xc3; break; case 0x0414: *str = 0xc4; break;
    case 0x0415: *str = 0xc5; break; case 0x0416: *str = 0xc6; break;
    case 0x0417: *str = 0xc7; break; case 0x0418: *str = 0xc8; break;
    case 0x0419: *str = 0xc9; break; case 0x041a: *str = 0xca; break;
    case 0x041b: *str = 0xcb; break; case 0x041c: *str = 0xcc; break;
    case 0x041d: *str = 0xcd; break; case 0x041e: *str = 0xce; break;
    case 0x041f: *str = 0xcf; break; case 0x0420: *str = 0xd0; break;
    case 0x0421: *str = 0xd1; break; case 0x0422: *str = 0xd2; break;
    case 0x0423: *str = 0xd3; break; case 0x0424: *str = 0xd4; break;
    case 0x0425: *str = 0xd5; break; case 0x0426: *str = 0xd6; break;
    case 0x0427: *str = 0xd7; break; case 0x0428: *str = 0xd8; break;
    case 0x0429: *str = 0xd9; break; case 0x042a: *str = 0xda; break;
    case 0x042b: *str = 0xdb; break; case 0x042c: *str = 0xdc; break;
    case 0x042d: *str = 0xdd; break; case 0x042e: *str = 0xde; break;
    case 0x042f: *str = 0xdf; break; case 0x0430: *str = 0xe0; break;
    case 0x0431: *str = 0xe1; break; case 0x0432: *str = 0xe2; break;
    case 0x0433: *str = 0xe3; break; case 0x0434: *str = 0xe4; break;
    case 0x0435: *str = 0xe5; break; case 0x0436: *str = 0xe6; break;
    case 0x0437: *str = 0xe7; break; case 0x0438: *str = 0xe8; break;
    case 0x0439: *str = 0xe9; break; case 0x043a: *str = 0xea; break;
    case 0x043b: *str = 0xeb; break; case 0x043c: *str = 0xec; break;
    case 0x043d: *str = 0xed; break; case 0x043e: *str = 0xee; break;
    case 0x043f: *str = 0xef; break; case 0x0440: *str = 0xf0; break;
    case 0x0441: *str = 0xf1; break; case 0x0442: *str = 0xf2; break;
    case 0x0443: *str = 0xf3; break; case 0x0444: *str = 0xf4; break;
    case 0x0445: *str = 0xf5; break; case 0x0446: *str = 0xf6; break;
    case 0x0447: *str = 0xf7; break; case 0x0448: *str = 0xf8; break;
    case 0x0449: *str = 0xf9; break; case 0x044a: *str = 0xfa; break;
    case 0x044b: *str = 0xfb; break; case 0x044c: *str = 0xfc; break;
    case 0x044d: *str = 0xfd; break; case 0x044e: *str = 0xfe; break;
    case 0x044f: *str = 0xff; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_1252_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1252_map);
}

u32 Nst_1252_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1252_map);
}

i32 Nst_1252_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x20ac: *str = 0x80; break; case 0x201a: *str = 0x82; break;
    case 0x0192: *str = 0x83; break; case 0x201e: *str = 0x84; break;
    case 0x2026: *str = 0x85; break; case 0x2020: *str = 0x86; break;
    case 0x2021: *str = 0x87; break; case 0x02c6: *str = 0x88; break;
    case 0x2030: *str = 0x89; break; case 0x0160: *str = 0x8a; break;
    case 0x2039: *str = 0x8b; break; case 0x0152: *str = 0x8c; break;
    case 0x017d: *str = 0x8e; break; case 0x2018: *str = 0x91; break;
    case 0x2019: *str = 0x92; break; case 0x201c: *str = 0x93; break;
    case 0x201d: *str = 0x94; break; case 0x2022: *str = 0x95; break;
    case 0x2013: *str = 0x96; break; case 0x2014: *str = 0x97; break;
    case 0x02dc: *str = 0x98; break; case 0x2122: *str = 0x99; break;
    case 0x0161: *str = 0x9a; break; case 0x203a: *str = 0x9b; break;
    case 0x0153: *str = 0x9c; break; case 0x017e: *str = 0x9e; break;
    case 0x0178: *str = 0x9f; break; case 0x00a0: *str = 0xa0; break;
    case 0x00a1: *str = 0xa1; break; case 0x00a2: *str = 0xa2; break;
    case 0x00a3: *str = 0xa3; break; case 0x00a4: *str = 0xa4; break;
    case 0x00a5: *str = 0xa5; break; case 0x00a6: *str = 0xa6; break;
    case 0x00a7: *str = 0xa7; break; case 0x00a8: *str = 0xa8; break;
    case 0x00a9: *str = 0xa9; break; case 0x00aa: *str = 0xaa; break;
    case 0x00ab: *str = 0xab; break; case 0x00ac: *str = 0xac; break;
    case 0x00ad: *str = 0xad; break; case 0x00ae: *str = 0xae; break;
    case 0x00af: *str = 0xaf; break; case 0x00b0: *str = 0xb0; break;
    case 0x00b1: *str = 0xb1; break; case 0x00b2: *str = 0xb2; break;
    case 0x00b3: *str = 0xb3; break; case 0x00b4: *str = 0xb4; break;
    case 0x00b5: *str = 0xb5; break; case 0x00b6: *str = 0xb6; break;
    case 0x00b7: *str = 0xb7; break; case 0x00b8: *str = 0xb8; break;
    case 0x00b9: *str = 0xb9; break; case 0x00ba: *str = 0xba; break;
    case 0x00bb: *str = 0xbb; break; case 0x00bc: *str = 0xbc; break;
    case 0x00bd: *str = 0xbd; break; case 0x00be: *str = 0xbe; break;
    case 0x00bf: *str = 0xbf; break; case 0x00c0: *str = 0xc0; break;
    case 0x00c1: *str = 0xc1; break; case 0x00c2: *str = 0xc2; break;
    case 0x00c3: *str = 0xc3; break; case 0x00c4: *str = 0xc4; break;
    case 0x00c5: *str = 0xc5; break; case 0x00c6: *str = 0xc6; break;
    case 0x00c7: *str = 0xc7; break; case 0x00c8: *str = 0xc8; break;
    case 0x00c9: *str = 0xc9; break; case 0x00ca: *str = 0xca; break;
    case 0x00cb: *str = 0xcb; break; case 0x00cc: *str = 0xcc; break;
    case 0x00cd: *str = 0xcd; break; case 0x00ce: *str = 0xce; break;
    case 0x00cf: *str = 0xcf; break; case 0x00d0: *str = 0xd0; break;
    case 0x00d1: *str = 0xd1; break; case 0x00d2: *str = 0xd2; break;
    case 0x00d3: *str = 0xd3; break; case 0x00d4: *str = 0xd4; break;
    case 0x00d5: *str = 0xd5; break; case 0x00d6: *str = 0xd6; break;
    case 0x00d7: *str = 0xd7; break; case 0x00d8: *str = 0xd8; break;
    case 0x00d9: *str = 0xd9; break; case 0x00da: *str = 0xda; break;
    case 0x00db: *str = 0xdb; break; case 0x00dc: *str = 0xdc; break;
    case 0x00dd: *str = 0xdd; break; case 0x00de: *str = 0xde; break;
    case 0x00df: *str = 0xdf; break; case 0x00e0: *str = 0xe0; break;
    case 0x00e1: *str = 0xe1; break; case 0x00e2: *str = 0xe2; break;
    case 0x00e3: *str = 0xe3; break; case 0x00e4: *str = 0xe4; break;
    case 0x00e5: *str = 0xe5; break; case 0x00e6: *str = 0xe6; break;
    case 0x00e7: *str = 0xe7; break; case 0x00e8: *str = 0xe8; break;
    case 0x00e9: *str = 0xe9; break; case 0x00ea: *str = 0xea; break;
    case 0x00eb: *str = 0xeb; break; case 0x00ec: *str = 0xec; break;
    case 0x00ed: *str = 0xed; break; case 0x00ee: *str = 0xee; break;
    case 0x00ef: *str = 0xef; break; case 0x00f0: *str = 0xf0; break;
    case 0x00f1: *str = 0xf1; break; case 0x00f2: *str = 0xf2; break;
    case 0x00f3: *str = 0xf3; break; case 0x00f4: *str = 0xf4; break;
    case 0x00f5: *str = 0xf5; break; case 0x00f6: *str = 0xf6; break;
    case 0x00f7: *str = 0xf7; break; case 0x00f8: *str = 0xf8; break;
    case 0x00f9: *str = 0xf9; break; case 0x00fa: *str = 0xfa; break;
    case 0x00fb: *str = 0xfb; break; case 0x00fc: *str = 0xfc; break;
    case 0x00fd: *str = 0xfd; break; case 0x00fe: *str = 0xfe; break;
    case 0x00ff: *str = 0xff; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_1253_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1253_map);
}

u32 Nst_1253_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1253_map);
}

i32 Nst_1253_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x20ac: *str = 0x80; break; case 0x201a: *str = 0x82; break;
    case 0x0192: *str = 0x83; break; case 0x201e: *str = 0x84; break;
    case 0x2026: *str = 0x85; break; case 0x2020: *str = 0x86; break;
    case 0x2021: *str = 0x87; break; case 0x2030: *str = 0x89; break;
    case 0x2039: *str = 0x8b; break; case 0x2018: *str = 0x91; break;
    case 0x2019: *str = 0x92; break; case 0x201c: *str = 0x93; break;
    case 0x201d: *str = 0x94; break; case 0x2022: *str = 0x95; break;
    case 0x2013: *str = 0x96; break; case 0x2014: *str = 0x97; break;
    case 0x2122: *str = 0x99; break; case 0x203a: *str = 0x9b; break;
    case 0x00a0: *str = 0xa0; break; case 0x0385: *str = 0xa1; break;
    case 0x0386: *str = 0xa2; break; case 0x00a3: *str = 0xa3; break;
    case 0x00a4: *str = 0xa4; break; case 0x00a5: *str = 0xa5; break;
    case 0x00a6: *str = 0xa6; break; case 0x00a7: *str = 0xa7; break;
    case 0x00a8: *str = 0xa8; break; case 0x00a9: *str = 0xa9; break;
    case 0x00ab: *str = 0xab; break; case 0x00ac: *str = 0xac; break;
    case 0x00ad: *str = 0xad; break; case 0x00ae: *str = 0xae; break;
    case 0x2015: *str = 0xaf; break; case 0x00b0: *str = 0xb0; break;
    case 0x00b1: *str = 0xb1; break; case 0x00b2: *str = 0xb2; break;
    case 0x00b3: *str = 0xb3; break; case 0x0384: *str = 0xb4; break;
    case 0x00b5: *str = 0xb5; break; case 0x00b6: *str = 0xb6; break;
    case 0x00b7: *str = 0xb7; break; case 0x0388: *str = 0xb8; break;
    case 0x0389: *str = 0xb9; break; case 0x038a: *str = 0xba; break;
    case 0x00bb: *str = 0xbb; break; case 0x038c: *str = 0xbc; break;
    case 0x00bd: *str = 0xbd; break; case 0x038e: *str = 0xbe; break;
    case 0x038f: *str = 0xbf; break; case 0x0390: *str = 0xc0; break;
    case 0x0391: *str = 0xc1; break; case 0x0392: *str = 0xc2; break;
    case 0x0393: *str = 0xc3; break; case 0x0394: *str = 0xc4; break;
    case 0x0395: *str = 0xc5; break; case 0x0396: *str = 0xc6; break;
    case 0x0397: *str = 0xc7; break; case 0x0398: *str = 0xc8; break;
    case 0x0399: *str = 0xc9; break; case 0x039a: *str = 0xca; break;
    case 0x039b: *str = 0xcb; break; case 0x039c: *str = 0xcc; break;
    case 0x039d: *str = 0xcd; break; case 0x039e: *str = 0xce; break;
    case 0x039f: *str = 0xcf; break; case 0x03a0: *str = 0xd0; break;
    case 0x03a1: *str = 0xd1; break; case 0x03a3: *str = 0xd3; break;
    case 0x03a4: *str = 0xd4; break; case 0x03a5: *str = 0xd5; break;
    case 0x03a6: *str = 0xd6; break; case 0x03a7: *str = 0xd7; break;
    case 0x03a8: *str = 0xd8; break; case 0x03a9: *str = 0xd9; break;
    case 0x03aa: *str = 0xda; break; case 0x03ab: *str = 0xdb; break;
    case 0x03ac: *str = 0xdc; break; case 0x03ad: *str = 0xdd; break;
    case 0x03ae: *str = 0xde; break; case 0x03af: *str = 0xdf; break;
    case 0x03b0: *str = 0xe0; break; case 0x03b1: *str = 0xe1; break;
    case 0x03b2: *str = 0xe2; break; case 0x03b3: *str = 0xe3; break;
    case 0x03b4: *str = 0xe4; break; case 0x03b5: *str = 0xe5; break;
    case 0x03b6: *str = 0xe6; break; case 0x03b7: *str = 0xe7; break;
    case 0x03b8: *str = 0xe8; break; case 0x03b9: *str = 0xe9; break;
    case 0x03ba: *str = 0xea; break; case 0x03bb: *str = 0xeb; break;
    case 0x03bc: *str = 0xec; break; case 0x03bd: *str = 0xed; break;
    case 0x03be: *str = 0xee; break; case 0x03bf: *str = 0xef; break;
    case 0x03c0: *str = 0xf0; break; case 0x03c1: *str = 0xf1; break;
    case 0x03c2: *str = 0xf2; break; case 0x03c3: *str = 0xf3; break;
    case 0x03c4: *str = 0xf4; break; case 0x03c5: *str = 0xf5; break;
    case 0x03c6: *str = 0xf6; break; case 0x03c7: *str = 0xf7; break;
    case 0x03c8: *str = 0xf8; break; case 0x03c9: *str = 0xf9; break;
    case 0x03ca: *str = 0xfa; break; case 0x03cb: *str = 0xfb; break;
    case 0x03cc: *str = 0xfc; break; case 0x03cd: *str = 0xfd; break;
    case 0x03ce: *str = 0xfe; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_1254_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1254_map);
}

u32 Nst_1254_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1254_map);
}

i32 Nst_1254_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x20ac: *str = 0x80; break; case 0x201a: *str = 0x82; break;
    case 0x0192: *str = 0x83; break; case 0x201e: *str = 0x84; break;
    case 0x2026: *str = 0x85; break; case 0x2020: *str = 0x86; break;
    case 0x2021: *str = 0x87; break; case 0x02c6: *str = 0x88; break;
    case 0x2030: *str = 0x89; break; case 0x0160: *str = 0x8a; break;
    case 0x2039: *str = 0x8b; break; case 0x0152: *str = 0x8c; break;
    case 0x2018: *str = 0x91; break; case 0x2019: *str = 0x92; break;
    case 0x201c: *str = 0x93; break; case 0x201d: *str = 0x94; break;
    case 0x2022: *str = 0x95; break; case 0x2013: *str = 0x96; break;
    case 0x2014: *str = 0x97; break; case 0x02dc: *str = 0x98; break;
    case 0x2122: *str = 0x99; break; case 0x0161: *str = 0x9a; break;
    case 0x203a: *str = 0x9b; break; case 0x0153: *str = 0x9c; break;
    case 0x0178: *str = 0x9f; break; case 0x00a0: *str = 0xa0; break;
    case 0x00a1: *str = 0xa1; break; case 0x00a2: *str = 0xa2; break;
    case 0x00a3: *str = 0xa3; break; case 0x00a4: *str = 0xa4; break;
    case 0x00a5: *str = 0xa5; break; case 0x00a6: *str = 0xa6; break;
    case 0x00a7: *str = 0xa7; break; case 0x00a8: *str = 0xa8; break;
    case 0x00a9: *str = 0xa9; break; case 0x00aa: *str = 0xaa; break;
    case 0x00ab: *str = 0xab; break; case 0x00ac: *str = 0xac; break;
    case 0x00ad: *str = 0xad; break; case 0x00ae: *str = 0xae; break;
    case 0x00af: *str = 0xaf; break; case 0x00b0: *str = 0xb0; break;
    case 0x00b1: *str = 0xb1; break; case 0x00b2: *str = 0xb2; break;
    case 0x00b3: *str = 0xb3; break; case 0x00b4: *str = 0xb4; break;
    case 0x00b5: *str = 0xb5; break; case 0x00b6: *str = 0xb6; break;
    case 0x00b7: *str = 0xb7; break; case 0x00b8: *str = 0xb8; break;
    case 0x00b9: *str = 0xb9; break; case 0x00ba: *str = 0xba; break;
    case 0x00bb: *str = 0xbb; break; case 0x00bc: *str = 0xbc; break;
    case 0x00bd: *str = 0xbd; break; case 0x00be: *str = 0xbe; break;
    case 0x00bf: *str = 0xbf; break; case 0x00c0: *str = 0xc0; break;
    case 0x00c1: *str = 0xc1; break; case 0x00c2: *str = 0xc2; break;
    case 0x00c3: *str = 0xc3; break; case 0x00c4: *str = 0xc4; break;
    case 0x00c5: *str = 0xc5; break; case 0x00c6: *str = 0xc6; break;
    case 0x00c7: *str = 0xc7; break; case 0x00c8: *str = 0xc8; break;
    case 0x00c9: *str = 0xc9; break; case 0x00ca: *str = 0xca; break;
    case 0x00cb: *str = 0xcb; break; case 0x00cc: *str = 0xcc; break;
    case 0x00cd: *str = 0xcd; break; case 0x00ce: *str = 0xce; break;
    case 0x00cf: *str = 0xcf; break; case 0x011e: *str = 0xd0; break;
    case 0x00d1: *str = 0xd1; break; case 0x00d2: *str = 0xd2; break;
    case 0x00d3: *str = 0xd3; break; case 0x00d4: *str = 0xd4; break;
    case 0x00d5: *str = 0xd5; break; case 0x00d6: *str = 0xd6; break;
    case 0x00d7: *str = 0xd7; break; case 0x00d8: *str = 0xd8; break;
    case 0x00d9: *str = 0xd9; break; case 0x00da: *str = 0xda; break;
    case 0x00db: *str = 0xdb; break; case 0x00dc: *str = 0xdc; break;
    case 0x0130: *str = 0xdd; break; case 0x015e: *str = 0xde; break;
    case 0x00df: *str = 0xdf; break; case 0x00e0: *str = 0xe0; break;
    case 0x00e1: *str = 0xe1; break; case 0x00e2: *str = 0xe2; break;
    case 0x00e3: *str = 0xe3; break; case 0x00e4: *str = 0xe4; break;
    case 0x00e5: *str = 0xe5; break; case 0x00e6: *str = 0xe6; break;
    case 0x00e7: *str = 0xe7; break; case 0x00e8: *str = 0xe8; break;
    case 0x00e9: *str = 0xe9; break; case 0x00ea: *str = 0xea; break;
    case 0x00eb: *str = 0xeb; break; case 0x00ec: *str = 0xec; break;
    case 0x00ed: *str = 0xed; break; case 0x00ee: *str = 0xee; break;
    case 0x00ef: *str = 0xef; break; case 0x011f: *str = 0xf0; break;
    case 0x00f1: *str = 0xf1; break; case 0x00f2: *str = 0xf2; break;
    case 0x00f3: *str = 0xf3; break; case 0x00f4: *str = 0xf4; break;
    case 0x00f5: *str = 0xf5; break; case 0x00f6: *str = 0xf6; break;
    case 0x00f7: *str = 0xf7; break; case 0x00f8: *str = 0xf8; break;
    case 0x00f9: *str = 0xf9; break; case 0x00fa: *str = 0xfa; break;
    case 0x00fb: *str = 0xfb; break; case 0x00fc: *str = 0xfc; break;
    case 0x0131: *str = 0xfd; break; case 0x015f: *str = 0xfe; break;
    case 0x00ff: *str = 0xff; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_1255_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1255_map);
}

u32 Nst_1255_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1255_map);
}

i32 Nst_1255_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x20ac: *str = 0x80; break; case 0x201a: *str = 0x82; break;
    case 0x0192: *str = 0x83; break; case 0x201e: *str = 0x84; break;
    case 0x2026: *str = 0x85; break; case 0x2020: *str = 0x86; break;
    case 0x2021: *str = 0x87; break; case 0x02c6: *str = 0x88; break;
    case 0x2030: *str = 0x89; break; case 0x2039: *str = 0x8b; break;
    case 0x2018: *str = 0x91; break; case 0x2019: *str = 0x92; break;
    case 0x201c: *str = 0x93; break; case 0x201d: *str = 0x94; break;
    case 0x2022: *str = 0x95; break; case 0x2013: *str = 0x96; break;
    case 0x2014: *str = 0x97; break; case 0x02dc: *str = 0x98; break;
    case 0x2122: *str = 0x99; break; case 0x203a: *str = 0x9b; break;
    case 0x00a0: *str = 0xa0; break; case 0x00a1: *str = 0xa1; break;
    case 0x00a2: *str = 0xa2; break; case 0x00a3: *str = 0xa3; break;
    case 0x20aa: *str = 0xa4; break; case 0x00a5: *str = 0xa5; break;
    case 0x00a6: *str = 0xa6; break; case 0x00a7: *str = 0xa7; break;
    case 0x00a8: *str = 0xa8; break; case 0x00a9: *str = 0xa9; break;
    case 0x00d7: *str = 0xaa; break; case 0x00ab: *str = 0xab; break;
    case 0x00ac: *str = 0xac; break; case 0x00ad: *str = 0xad; break;
    case 0x00ae: *str = 0xae; break; case 0x00af: *str = 0xaf; break;
    case 0x00b0: *str = 0xb0; break; case 0x00b1: *str = 0xb1; break;
    case 0x00b2: *str = 0xb2; break; case 0x00b3: *str = 0xb3; break;
    case 0x00b4: *str = 0xb4; break; case 0x00b5: *str = 0xb5; break;
    case 0x00b6: *str = 0xb6; break; case 0x00b7: *str = 0xb7; break;
    case 0x00b8: *str = 0xb8; break; case 0x00b9: *str = 0xb9; break;
    case 0x00f7: *str = 0xba; break; case 0x00bb: *str = 0xbb; break;
    case 0x00bc: *str = 0xbc; break; case 0x00bd: *str = 0xbd; break;
    case 0x00be: *str = 0xbe; break; case 0x00bf: *str = 0xbf; break;
    case 0x05b0: *str = 0xc0; break; case 0x05b1: *str = 0xc1; break;
    case 0x05b2: *str = 0xc2; break; case 0x05b3: *str = 0xc3; break;
    case 0x05b4: *str = 0xc4; break; case 0x05b5: *str = 0xc5; break;
    case 0x05b6: *str = 0xc6; break; case 0x05b7: *str = 0xc7; break;
    case 0x05b8: *str = 0xc8; break; case 0x05b9: *str = 0xc9; break;
    case 0x05bb: *str = 0xcb; break; case 0x05bc: *str = 0xcc; break;
    case 0x05bd: *str = 0xcd; break; case 0x05be: *str = 0xce; break;
    case 0x05bf: *str = 0xcf; break; case 0x05c0: *str = 0xd0; break;
    case 0x05c1: *str = 0xd1; break; case 0x05c2: *str = 0xd2; break;
    case 0x05c3: *str = 0xd3; break; case 0x05f0: *str = 0xd4; break;
    case 0x05f1: *str = 0xd5; break; case 0x05f2: *str = 0xd6; break;
    case 0x05f3: *str = 0xd7; break; case 0x05f4: *str = 0xd8; break;
    case 0x05d0: *str = 0xe0; break; case 0x05d1: *str = 0xe1; break;
    case 0x05d2: *str = 0xe2; break; case 0x05d3: *str = 0xe3; break;
    case 0x05d4: *str = 0xe4; break; case 0x05d5: *str = 0xe5; break;
    case 0x05d6: *str = 0xe6; break; case 0x05d7: *str = 0xe7; break;
    case 0x05d8: *str = 0xe8; break; case 0x05d9: *str = 0xe9; break;
    case 0x05da: *str = 0xea; break; case 0x05db: *str = 0xeb; break;
    case 0x05dc: *str = 0xec; break; case 0x05dd: *str = 0xed; break;
    case 0x05de: *str = 0xee; break; case 0x05df: *str = 0xef; break;
    case 0x05e0: *str = 0xf0; break; case 0x05e1: *str = 0xf1; break;
    case 0x05e2: *str = 0xf2; break; case 0x05e3: *str = 0xf3; break;
    case 0x05e4: *str = 0xf4; break; case 0x05e5: *str = 0xf5; break;
    case 0x05e6: *str = 0xf6; break; case 0x05e7: *str = 0xf7; break;
    case 0x05e8: *str = 0xf8; break; case 0x05e9: *str = 0xf9; break;
    case 0x05ea: *str = 0xfa; break; case 0x200e: *str = 0xfd; break;
    case 0x200f: *str = 0xfe; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_1256_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1256_map);
}

u32 Nst_1256_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1256_map);
}

i32 Nst_1256_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x20ac: *str = 0x80; break; case 0x067e: *str = 0x81; break;
    case 0x201a: *str = 0x82; break; case 0x0192: *str = 0x83; break;
    case 0x201e: *str = 0x84; break; case 0x2026: *str = 0x85; break;
    case 0x2020: *str = 0x86; break; case 0x2021: *str = 0x87; break;
    case 0x02c6: *str = 0x88; break; case 0x2030: *str = 0x89; break;
    case 0x0679: *str = 0x8a; break; case 0x2039: *str = 0x8b; break;
    case 0x0152: *str = 0x8c; break; case 0x0686: *str = 0x8d; break;
    case 0x0698: *str = 0x8e; break; case 0x0688: *str = 0x8f; break;
    case 0x06af: *str = 0x90; break; case 0x2018: *str = 0x91; break;
    case 0x2019: *str = 0x92; break; case 0x201c: *str = 0x93; break;
    case 0x201d: *str = 0x94; break; case 0x2022: *str = 0x95; break;
    case 0x2013: *str = 0x96; break; case 0x2014: *str = 0x97; break;
    case 0x06a9: *str = 0x98; break; case 0x2122: *str = 0x99; break;
    case 0x0691: *str = 0x9a; break; case 0x203a: *str = 0x9b; break;
    case 0x0153: *str = 0x9c; break; case 0x200c: *str = 0x9d; break;
    case 0x200d: *str = 0x9e; break; case 0x06ba: *str = 0x9f; break;
    case 0x00a0: *str = 0xa0; break; case 0x060c: *str = 0xa1; break;
    case 0x00a2: *str = 0xa2; break; case 0x00a3: *str = 0xa3; break;
    case 0x00a4: *str = 0xa4; break; case 0x00a5: *str = 0xa5; break;
    case 0x00a6: *str = 0xa6; break; case 0x00a7: *str = 0xa7; break;
    case 0x00a8: *str = 0xa8; break; case 0x00a9: *str = 0xa9; break;
    case 0x06be: *str = 0xaa; break; case 0x00ab: *str = 0xab; break;
    case 0x00ac: *str = 0xac; break; case 0x00ad: *str = 0xad; break;
    case 0x00ae: *str = 0xae; break; case 0x00af: *str = 0xaf; break;
    case 0x00b0: *str = 0xb0; break; case 0x00b1: *str = 0xb1; break;
    case 0x00b2: *str = 0xb2; break; case 0x00b3: *str = 0xb3; break;
    case 0x00b4: *str = 0xb4; break; case 0x00b5: *str = 0xb5; break;
    case 0x00b6: *str = 0xb6; break; case 0x00b7: *str = 0xb7; break;
    case 0x00b8: *str = 0xb8; break; case 0x00b9: *str = 0xb9; break;
    case 0x061b: *str = 0xba; break; case 0x00bb: *str = 0xbb; break;
    case 0x00bc: *str = 0xbc; break; case 0x00bd: *str = 0xbd; break;
    case 0x00be: *str = 0xbe; break; case 0x061f: *str = 0xbf; break;
    case 0x06c1: *str = 0xc0; break; case 0x0621: *str = 0xc1; break;
    case 0x0622: *str = 0xc2; break; case 0x0623: *str = 0xc3; break;
    case 0x0624: *str = 0xc4; break; case 0x0625: *str = 0xc5; break;
    case 0x0626: *str = 0xc6; break; case 0x0627: *str = 0xc7; break;
    case 0x0628: *str = 0xc8; break; case 0x0629: *str = 0xc9; break;
    case 0x062a: *str = 0xca; break; case 0x062b: *str = 0xcb; break;
    case 0x062c: *str = 0xcc; break; case 0x062d: *str = 0xcd; break;
    case 0x062e: *str = 0xce; break; case 0x062f: *str = 0xcf; break;
    case 0x0630: *str = 0xd0; break; case 0x0631: *str = 0xd1; break;
    case 0x0632: *str = 0xd2; break; case 0x0633: *str = 0xd3; break;
    case 0x0634: *str = 0xd4; break; case 0x0635: *str = 0xd5; break;
    case 0x0636: *str = 0xd6; break; case 0x00d7: *str = 0xd7; break;
    case 0x0637: *str = 0xd8; break; case 0x0638: *str = 0xd9; break;
    case 0x0639: *str = 0xda; break; case 0x063a: *str = 0xdb; break;
    case 0x0640: *str = 0xdc; break; case 0x0641: *str = 0xdd; break;
    case 0x0642: *str = 0xde; break; case 0x0643: *str = 0xdf; break;
    case 0x00e0: *str = 0xe0; break; case 0x0644: *str = 0xe1; break;
    case 0x00e2: *str = 0xe2; break; case 0x0645: *str = 0xe3; break;
    case 0x0646: *str = 0xe4; break; case 0x0647: *str = 0xe5; break;
    case 0x0648: *str = 0xe6; break; case 0x00e7: *str = 0xe7; break;
    case 0x00e8: *str = 0xe8; break; case 0x00e9: *str = 0xe9; break;
    case 0x00ea: *str = 0xea; break; case 0x00eb: *str = 0xeb; break;
    case 0x0649: *str = 0xec; break; case 0x064a: *str = 0xed; break;
    case 0x00ee: *str = 0xee; break; case 0x00ef: *str = 0xef; break;
    case 0x064b: *str = 0xf0; break; case 0x064c: *str = 0xf1; break;
    case 0x064d: *str = 0xf2; break; case 0x064e: *str = 0xf3; break;
    case 0x00f4: *str = 0xf4; break; case 0x064f: *str = 0xf5; break;
    case 0x0650: *str = 0xf6; break; case 0x00f7: *str = 0xf7; break;
    case 0x0651: *str = 0xf8; break; case 0x00f9: *str = 0xf9; break;
    case 0x0652: *str = 0xfa; break; case 0x00fb: *str = 0xfb; break;
    case 0x00fc: *str = 0xfc; break; case 0x200e: *str = 0xfd; break;
    case 0x200f: *str = 0xfe; break; case 0x06d2: *str = 0xff; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_1257_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1257_map);
}

u32 Nst_1257_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1257_map);
}

i32 Nst_1257_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x20ac: *str = 0x80; break; case 0x201a: *str = 0x82; break;
    case 0x201e: *str = 0x84; break; case 0x2026: *str = 0x85; break;
    case 0x2020: *str = 0x86; break; case 0x2021: *str = 0x87; break;
    case 0x2030: *str = 0x89; break; case 0x2039: *str = 0x8b; break;
    case 0x00a8: *str = 0x8d; break; case 0x02c7: *str = 0x8e; break;
    case 0x00b8: *str = 0x8f; break; case 0x2018: *str = 0x91; break;
    case 0x2019: *str = 0x92; break; case 0x201c: *str = 0x93; break;
    case 0x201d: *str = 0x94; break; case 0x2022: *str = 0x95; break;
    case 0x2013: *str = 0x96; break; case 0x2014: *str = 0x97; break;
    case 0x2122: *str = 0x99; break; case 0x203a: *str = 0x9b; break;
    case 0x00af: *str = 0x9d; break; case 0x02db: *str = 0x9e; break;
    case 0x00a0: *str = 0xa0; break; case 0x00a2: *str = 0xa2; break;
    case 0x00a3: *str = 0xa3; break; case 0x00a4: *str = 0xa4; break;
    case 0x00a6: *str = 0xa6; break; case 0x00a7: *str = 0xa7; break;
    case 0x00d8: *str = 0xa8; break; case 0x00a9: *str = 0xa9; break;
    case 0x0156: *str = 0xaa; break; case 0x00ab: *str = 0xab; break;
    case 0x00ac: *str = 0xac; break; case 0x00ad: *str = 0xad; break;
    case 0x00ae: *str = 0xae; break; case 0x00c6: *str = 0xaf; break;
    case 0x00b0: *str = 0xb0; break; case 0x00b1: *str = 0xb1; break;
    case 0x00b2: *str = 0xb2; break; case 0x00b3: *str = 0xb3; break;
    case 0x00b4: *str = 0xb4; break; case 0x00b5: *str = 0xb5; break;
    case 0x00b6: *str = 0xb6; break; case 0x00b7: *str = 0xb7; break;
    case 0x00f8: *str = 0xb8; break; case 0x00b9: *str = 0xb9; break;
    case 0x0157: *str = 0xba; break; case 0x00bb: *str = 0xbb; break;
    case 0x00bc: *str = 0xbc; break; case 0x00bd: *str = 0xbd; break;
    case 0x00be: *str = 0xbe; break; case 0x00e6: *str = 0xbf; break;
    case 0x0104: *str = 0xc0; break; case 0x012e: *str = 0xc1; break;
    case 0x0100: *str = 0xc2; break; case 0x0106: *str = 0xc3; break;
    case 0x00c4: *str = 0xc4; break; case 0x00c5: *str = 0xc5; break;
    case 0x0118: *str = 0xc6; break; case 0x0112: *str = 0xc7; break;
    case 0x010c: *str = 0xc8; break; case 0x00c9: *str = 0xc9; break;
    case 0x0179: *str = 0xca; break; case 0x0116: *str = 0xcb; break;
    case 0x0122: *str = 0xcc; break; case 0x0136: *str = 0xcd; break;
    case 0x012a: *str = 0xce; break; case 0x013b: *str = 0xcf; break;
    case 0x0160: *str = 0xd0; break; case 0x0143: *str = 0xd1; break;
    case 0x0145: *str = 0xd2; break; case 0x00d3: *str = 0xd3; break;
    case 0x014c: *str = 0xd4; break; case 0x00d5: *str = 0xd5; break;
    case 0x00d6: *str = 0xd6; break; case 0x00d7: *str = 0xd7; break;
    case 0x0172: *str = 0xd8; break; case 0x0141: *str = 0xd9; break;
    case 0x015a: *str = 0xda; break; case 0x016a: *str = 0xdb; break;
    case 0x00dc: *str = 0xdc; break; case 0x017b: *str = 0xdd; break;
    case 0x017d: *str = 0xde; break; case 0x00df: *str = 0xdf; break;
    case 0x0105: *str = 0xe0; break; case 0x012f: *str = 0xe1; break;
    case 0x0101: *str = 0xe2; break; case 0x0107: *str = 0xe3; break;
    case 0x00e4: *str = 0xe4; break; case 0x00e5: *str = 0xe5; break;
    case 0x0119: *str = 0xe6; break; case 0x0113: *str = 0xe7; break;
    case 0x010d: *str = 0xe8; break; case 0x00e9: *str = 0xe9; break;
    case 0x017a: *str = 0xea; break; case 0x0117: *str = 0xeb; break;
    case 0x0123: *str = 0xec; break; case 0x0137: *str = 0xed; break;
    case 0x012b: *str = 0xee; break; case 0x013c: *str = 0xef; break;
    case 0x0161: *str = 0xf0; break; case 0x0144: *str = 0xf1; break;
    case 0x0146: *str = 0xf2; break; case 0x00f3: *str = 0xf3; break;
    case 0x014d: *str = 0xf4; break; case 0x00f5: *str = 0xf5; break;
    case 0x00f6: *str = 0xf6; break; case 0x00f7: *str = 0xf7; break;
    case 0x0173: *str = 0xf8; break; case 0x0142: *str = 0xf9; break;
    case 0x015b: *str = 0xfa; break; case 0x016b: *str = 0xfb; break;
    case 0x00fc: *str = 0xfc; break; case 0x017c: *str = 0xfd; break;
    case 0x017e: *str = 0xfe; break; case 0x02d9: *str = 0xff; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_1258_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    return Nst_check_125x_bytes(str, cp_1258_map);
}

u32 Nst_1258_to_utf32(u8 *str)
{
    return Nst_125x_to_utf32(str, cp_1258_map);
}

i32 Nst_1258_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }

    switch (ch) {
    case 0x20ac: *str = 0x80; break; case 0x201a: *str = 0x82; break;
    case 0x0192: *str = 0x83; break; case 0x201e: *str = 0x84; break;
    case 0x2026: *str = 0x85; break; case 0x2020: *str = 0x86; break;
    case 0x2021: *str = 0x87; break; case 0x02c6: *str = 0x88; break;
    case 0x2030: *str = 0x89; break; case 0x2039: *str = 0x8b; break;
    case 0x0152: *str = 0x8c; break; case 0x2018: *str = 0x91; break;
    case 0x2019: *str = 0x92; break; case 0x201c: *str = 0x93; break;
    case 0x201d: *str = 0x94; break; case 0x2022: *str = 0x95; break;
    case 0x2013: *str = 0x96; break; case 0x2014: *str = 0x97; break;
    case 0x02dc: *str = 0x98; break; case 0x2122: *str = 0x99; break;
    case 0x203a: *str = 0x9b; break; case 0x0153: *str = 0x9c; break;
    case 0x0178: *str = 0x9f; break; case 0x00a0: *str = 0xa0; break;
    case 0x00a1: *str = 0xa1; break; case 0x00a2: *str = 0xa2; break;
    case 0x00a3: *str = 0xa3; break; case 0x00a4: *str = 0xa4; break;
    case 0x00a5: *str = 0xa5; break; case 0x00a6: *str = 0xa6; break;
    case 0x00a7: *str = 0xa7; break; case 0x00a8: *str = 0xa8; break;
    case 0x00a9: *str = 0xa9; break; case 0x00aa: *str = 0xaa; break;
    case 0x00ab: *str = 0xab; break; case 0x00ac: *str = 0xac; break;
    case 0x00ad: *str = 0xad; break; case 0x00ae: *str = 0xae; break;
    case 0x00af: *str = 0xaf; break; case 0x00b0: *str = 0xb0; break;
    case 0x00b1: *str = 0xb1; break; case 0x00b2: *str = 0xb2; break;
    case 0x00b3: *str = 0xb3; break; case 0x00b4: *str = 0xb4; break;
    case 0x00b5: *str = 0xb5; break; case 0x00b6: *str = 0xb6; break;
    case 0x00b7: *str = 0xb7; break; case 0x00b8: *str = 0xb8; break;
    case 0x00b9: *str = 0xb9; break; case 0x00ba: *str = 0xba; break;
    case 0x00bb: *str = 0xbb; break; case 0x00bc: *str = 0xbc; break;
    case 0x00bd: *str = 0xbd; break; case 0x00be: *str = 0xbe; break;
    case 0x00bf: *str = 0xbf; break; case 0x00c0: *str = 0xc0; break;
    case 0x00c1: *str = 0xc1; break; case 0x00c2: *str = 0xc2; break;
    case 0x0102: *str = 0xc3; break; case 0x00c4: *str = 0xc4; break;
    case 0x00c5: *str = 0xc5; break; case 0x00c6: *str = 0xc6; break;
    case 0x00c7: *str = 0xc7; break; case 0x00c8: *str = 0xc8; break;
    case 0x00c9: *str = 0xc9; break; case 0x00ca: *str = 0xca; break;
    case 0x00cb: *str = 0xcb; break; case 0x0300: *str = 0xcc; break;
    case 0x00cd: *str = 0xcd; break; case 0x00ce: *str = 0xce; break;
    case 0x00cf: *str = 0xcf; break; case 0x0110: *str = 0xd0; break;
    case 0x00d1: *str = 0xd1; break; case 0x0309: *str = 0xd2; break;
    case 0x00d3: *str = 0xd3; break; case 0x00d4: *str = 0xd4; break;
    case 0x01a0: *str = 0xd5; break; case 0x00d6: *str = 0xd6; break;
    case 0x00d7: *str = 0xd7; break; case 0x00d8: *str = 0xd8; break;
    case 0x00d9: *str = 0xd9; break; case 0x00da: *str = 0xda; break;
    case 0x00db: *str = 0xdb; break; case 0x00dc: *str = 0xdc; break;
    case 0x01af: *str = 0xdd; break; case 0x0303: *str = 0xde; break;
    case 0x00df: *str = 0xdf; break; case 0x00e0: *str = 0xe0; break;
    case 0x00e1: *str = 0xe1; break; case 0x00e2: *str = 0xe2; break;
    case 0x0103: *str = 0xe3; break; case 0x00e4: *str = 0xe4; break;
    case 0x00e5: *str = 0xe5; break; case 0x00e6: *str = 0xe6; break;
    case 0x00e7: *str = 0xe7; break; case 0x00e8: *str = 0xe8; break;
    case 0x00e9: *str = 0xe9; break; case 0x00ea: *str = 0xea; break;
    case 0x00eb: *str = 0xeb; break; case 0x0301: *str = 0xec; break;
    case 0x00ed: *str = 0xed; break; case 0x00ee: *str = 0xee; break;
    case 0x00ef: *str = 0xef; break; case 0x0111: *str = 0xf0; break;
    case 0x00f1: *str = 0xf1; break; case 0x0323: *str = 0xf2; break;
    case 0x00f3: *str = 0xf3; break; case 0x00f4: *str = 0xf4; break;
    case 0x01a1: *str = 0xf5; break; case 0x00f6: *str = 0xf6; break;
    case 0x00f7: *str = 0xf7; break; case 0x00f8: *str = 0xf8; break;
    case 0x00f9: *str = 0xf9; break; case 0x00fa: *str = 0xfa; break;
    case 0x00fb: *str = 0xfb; break; case 0x00fc: *str = 0xfc; break;
    case 0x01b0: *str = 0xfd; break; case 0x20ab: *str = 0xfe; break;
    case 0x00ff: *str = 0xff; break;
    default: return -1;
    }

    return 1;
}

i32 Nst_check_iso8859_1_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    u8 ch = *str;
    if (ch < 0x20 || (ch > 0x7e && ch < 0xa0))
        return -1;
    return 1;
}

u32 Nst_iso8859_1_to_utf32(u8 *str)
{
    return (u32)*str;
}

i32 Nst_iso8859_1_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x20 || (ch > 0x7e && ch < 0xa0) || ch > 0xff)
        return -1;
    *str = (u8)ch;
    return 1;
}

i32 Nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len)
{
    if (Nst_check_utf16_bytes(in_str, in_str_len) < 0)
        return -1;

    u32 n = Nst_utf16_to_utf32(in_str);
    return Nst_utf8_from_utf32(n, (u8 *)out_str);
}

bool Nst_translate_cp(Nst_CP *from, Nst_CP *to, void *from_buf, usize from_len,
                      void **to_buf, usize *to_len)
{
    if (to_len != NULL)
        *to_len = 0;
    *to_buf = NULL;

    // copy the string if the encoding is the same
    if (from == to) {
        *to_buf = (i8 *)Nst_malloc(from_len + 1, from->ch_size);
        if (*to_buf == NULL) {
            return false;
        }
        memcpy(*to_buf, from_buf, from->ch_size * (from_len + 1));
        if (to_len != NULL)
            *to_len = from_len;
        return true;
    }

    Nst_Buffer buf;
    if (!Nst_buffer_init(&buf, from_len * to->mult_min_sz + 40 + to->bom_size))
        return false;

    // skip BOM of initial string
    if (from->bom != NULL && from_len >= from->bom_size) {
        if (memcmp(from->bom, from_buf, from->bom_size) == 0)
            from_buf = (void *)((i8 *)from_buf + from->bom_size);
    }

    isize n = (isize)from_len; // avoids accidental underflow
    while (n > 0) {
        // Decode character
        i32 ch_len = from->check_bytes(from_buf, n);
        if (ch_len < 0) {
            Nst_buffer_destroy(&buf);
            Nst_set_value_error(Nst_sprintf(
                _Nst_EM_INVALID_ENCODING,
                *(u8 *)from_buf, from->name));
            return false;
        }
        usize ch_size = ch_len * from->ch_size;
        u32 utf32_ch = from->to_utf32(from_buf);
        from_buf = (u8 *)from_buf + ch_size;
        n -= ch_len;

        // Re-encode character
        if (!Nst_buffer_expand_by(&buf, to->mult_max_sz + to->ch_size)) {
            Nst_buffer_destroy(&buf);
            return false;
        }
        ch_len = to->from_utf32(utf32_ch, buf.data + buf.len);
        if (ch_len < 0) {
            Nst_buffer_destroy(&buf);
            Nst_set_value_error(Nst_sprintf(
                _Nst_EM_INVALID_DECODING,
                (int)utf32_ch, from->name));
        }
        buf.len += ch_len * to->ch_size;
    }
    if (!Nst_buffer_expand_by(&buf, to->ch_size)) {
        Nst_buffer_destroy(&buf);
        return false;
    }
    memset(buf.data + buf.len, 0, to->ch_size);

    *to_buf = buf.data;
    if (to_len != NULL)
        *to_len = buf.len;
    return true;
}

isize Nst_check_string_cp(Nst_CP *cp, void *str, usize str_len)
{
    Nst_CheckBytesFunc cp_check_bytes = cp->check_bytes;
    usize cp_ch_size = cp->ch_size;
    for (usize i = 0; i < str_len; i++) {
        i32 ch_len = cp_check_bytes(str, str_len - i);
        if (ch_len < 0)
            return i;
        usize ch_size = ch_len * cp_ch_size;
        str = (u8 *)str + ch_size;
        i += ch_len - 1;
    }
    return -1;
}

isize Nst_string_char_len(Nst_CP *cp, void *str, usize str_len)
{
    Nst_CheckBytesFunc cp_check_bytes = cp->check_bytes;
    usize cp_ch_size = cp->ch_size;
    isize len = 0;
    for (usize i = 0; i < str_len; i++) {
        i32 ch_len = cp_check_bytes(str, str_len - i);
        if (ch_len < 0) {
            Nst_set_value_errorf(
                _Nst_EM_INVALID_ENCODING,
                *(u8 *)str, cp->name);
            return -1;
        }
        usize ch_size = ch_len * cp_ch_size;
        str = (u8 *)str + ch_size;
        i += ch_len - 1;
        len++;
    }
    return len;
}

usize Nst_string_utf8_char_len(u8 *str, usize str_len)
{
    usize len = 0;
    u8 *s_end = str + str_len;
    str--;
    while (++str < s_end) {
        len++;
        if (!(*str & 0b10000000))
            continue;
        u8 bits = (*str & 0b00110000) >> 4;
        if (!(bits & 0b10))
            str += 1;
        else
            str += bits;
    }
    return len;
}

Nst_CP *Nst_cp(Nst_CPID cpid)
{
    switch (cpid) {
    case Nst_CP_ASCII:   return &Nst_cp_ascii;
    case Nst_CP_UTF8:    return &Nst_cp_utf8;
    case Nst_CP_EXT_UTF8:return &Nst_cp_ext_utf8;
    case Nst_CP_UTF16:   return &Nst_cp_utf16;
    case Nst_CP_UTF16BE: return &Nst_cp_utf16be;
    case Nst_CP_UTF16LE: return &Nst_cp_utf16le;
    case Nst_CP_UTF32:   return &Nst_cp_utf32;
    case Nst_CP_UTF32BE: return &Nst_cp_utf32be;
    case Nst_CP_UTF32LE: return &Nst_cp_utf32le;
    case Nst_CP_1250:    return &Nst_cp_1250;
    case Nst_CP_1251:    return &Nst_cp_1251;
    case Nst_CP_1252:    return &Nst_cp_1252;
    case Nst_CP_1253:    return &Nst_cp_1253;
    case Nst_CP_1254:    return &Nst_cp_1254;
    case Nst_CP_1255:    return &Nst_cp_1255;
    case Nst_CP_1256:    return &Nst_cp_1256;
    case Nst_CP_1257:    return &Nst_cp_1257;
    case Nst_CP_1258:    return &Nst_cp_1258;
    case Nst_CP_LATIN1:  return &Nst_cp_iso8859_1;
    case Nst_CP_UNKNOWN:
    default: return NULL;
    }
}

#ifdef Nst_WIN

Nst_CPID Nst_acp(void)
{
    UINT acp = GetACP();
    switch (acp) {
    case 1250: return Nst_CP_1250;
    case 1251: return Nst_CP_1251;
    case 1252: return Nst_CP_1252;
    case 1253: return Nst_CP_1253;
    case 1254: return Nst_CP_1254;
    case 1255: return Nst_CP_1255;
    case 1256: return Nst_CP_1256;
    case 1257: return Nst_CP_1257;
    case 1258: return Nst_CP_1258;
    // Windows supports unpaired surrogates so it is better to use the extended
    // UTF16 encodings
    case 1200: return Nst_CP_EXT_UTF16LE;
    case 1201: return Nst_CP_EXT_UTF16BE;
    case 12000: return Nst_CP_UTF32LE;
    case 12001: return Nst_CP_UTF32BE;
    case 20127: return Nst_CP_ASCII;
    case 65001: return Nst_CP_UTF8;
    default: return Nst_CP_ISO8859_1;
    }
}

#endif // !Nst_WIN

wchar_t *Nst_char_to_wchar_t(i8 *str, usize len)
{
    wchar_t *out_str;
    if (len == 0)
        len = strlen(str);

    bool res = Nst_translate_cp(
        &Nst_cp_ext_utf8,
        &Nst_cp_ext_utf16,
        (void *)str, len,
        (void **)&out_str, NULL);
    if (res == false)
        return NULL;
    return out_str;
}

i8 *Nst_wchar_t_to_char(wchar_t *str, usize len)
{
    i8 *out_str;
    if (len == 0)
        len = wcslen(str);

    bool res = Nst_translate_cp(
        &Nst_cp_ext_utf16,
        &Nst_cp_ext_utf8,
        (void *)str, len,
        (void **)&out_str, NULL);
    if (res == false)
        return NULL;
    return out_str;
}

bool Nst_is_valid_cp(u32 cp)
{
    return cp <= 0x10ffff && (cp < 0xd800 || cp > 0xdfff);
}

bool Nst_is_non_character(u32 cp)
{
    return (cp > 0xfdd0 && cp < 0xfdef)
        || ((cp & 0xfff0) == 0xfff0 && (cp & 0xf) > 0xe);
}

Nst_CPID Nst_check_bom(i8 *str, usize len, i32 *bom_size)
{
    Nst_CPID cpid = Nst_CP_UNKNOWN;
    i32 size = 0;
    u8 *us = (u8 *)str;

    if (len >= 4) {
        if (us[0] == 0 && us[1] == 0 && us[2] == 0xfe && us[3] == 0xff) {
            cpid = Nst_CP_UTF32BE;
            size = 4;
            goto end;
        } else if (us[0] == 0xff && us[1] == 0xfe && us[2] == 0 && us[3] == 0) {
            cpid = Nst_CP_UTF32LE;
            size = 4;
            goto end;
        }
    }

    if (len >= 3 && us[0] == 0xef && us[1] == 0xbb && us[2] == 0xbf) {
        cpid = Nst_CP_UTF8;
        size = 3;
        goto end;
    }

    if (len >= 2) {
        if (us[0] == 0xfe && us[1] == 0xff) {
            cpid = Nst_CP_UTF16BE;
            size = 2;
            goto end;
        } else if (us[0] == 0xff && us[1] == 0xfe) {
            cpid = Nst_CP_UTF16LE;
            size = 2;
            goto end;
        }
    }

end:
    if (bom_size != NULL)
        *bom_size = size;
    return cpid;
}

Nst_CPID Nst_detect_encoding(i8 *str, usize len, i32 *bom_size)
{
    Nst_CPID cpid = Nst_check_bom(str, len, bom_size);
    if (cpid != Nst_CP_UNKNOWN)
        return cpid;

    // The encoding is checked as follows:
    // 1. Test for UTF-8
    // 2. Test for UTF-16LE on little endian and UTF-16BE on big endian systems
    // 3. If 1 and 2 fail and it's on Windows check the local ansi CP
    // 4. Default to Latin-1

    isize res = Nst_check_string_cp(&Nst_cp_utf8, str, len);
    if (res == -1)
        return Nst_CP_UTF8;

#if Nst_BYTEORDER == Nst_LITTLE_ENDIAN
    res = Nst_check_string_cp(&Nst_cp_utf16le, str, len);
#else
    res = Nst_check_string_cp(&Nst_cp_utf16be, str, len);
#endif

    if (res == -1)
        return Nst_CP_UTF16LE;

#ifdef Nst_WIN
    cpid = Nst_acp();
    if (cpid != Nst_CP_UTF8 && cpid != Nst_CP_ISO8859_1) {
        res = Nst_check_string_cp(Nst_cp(cpid), str, len);
        if (res == -1)
            return cpid;
    }
#endif // !Nst_WIN

    return Nst_CP_ISO8859_1;
}

Nst_CPID Nst_encoding_from_name(i8 *name)
{
    usize name_len = strlen(name);
    if (name_len > 15)
        return Nst_CP_UNKNOWN;
    i8 name_cpy[16];
    i8 *name_p = name_cpy;

    for (usize i = 0; i < name_len; i++) {
        name_cpy[i] = (i8)tolower((u8)name[i]);
        if (name_cpy[i] == '_')
            name_cpy[i] = '-';
        else if (name_cpy[i] == ' ')
            name_cpy[i] = '-';
    }
    name_cpy[name_len] = 0;

    if (name_len > 3 && strncmp(name_p, "utf", 3) == 0) {
        name_p += 3;
        if (strcmp(name_p, "8") == 0 || strcmp(name_p, "-8") == 0)
            return Nst_CP_UTF8;
        if (strcmp(name_p, "16") == 0 || strcmp(name_p, "-16") == 0
            || strcmp(name_p, "16le") == 0 || strcmp(name_p, "-16le") == 0)
        {
            return Nst_CP_UTF16LE;
        }
        if (strcmp(name_p, "16be") == 0 || strcmp(name_p, "-16be") == 0)
            return Nst_CP_UTF16BE;
        if (strcmp(name_p, "32") == 0 || strcmp(name_p, "-32") == 0
            || strcmp(name_p, "32le") == 0 || strcmp(name_p, "-32le") == 0)
        {
            return Nst_CP_UTF32LE;
        }
        if (strcmp(name_p, "32be") == 0 || strcmp(name_p, "-32be") == 0)
            return Nst_CP_UTF32BE;
        return Nst_CP_UNKNOWN;
    }

    if (strncmp(name_p, "ext-utf", 7) == 0
        || strncmp(name_p, "extutf", 6) == 0)
    {
        name_p += name_p[3] == '-' ? 7 : 6;
        if (strcmp(name_p, "8") == 0 || strcmp(name_p, "-8") == 0)
            return Nst_CP_EXT_UTF8;
        if (strcmp(name_p, "16") == 0 || strcmp(name_p, "-16") == 0
            || strcmp(name_p, "16le") == 0 || strcmp(name_p, "-16le") == 0)
        {
            return Nst_CP_EXT_UTF16LE;
        }
        if (strcmp(name_p, "16be") == 0 || strcmp(name_p, "-16be") == 0)
            return Nst_CP_EXT_UTF16BE;
    }

    if ((name_len > 2 && strncmp(name_p, "cp", 2) == 0)
        || (name_len > 7 && strncmp(name_p, "windows", 7) == 0))
    {
        if (name_p[0] == 'c')
            name_p = name_p + 2;
        else
            name_p = name_p + 7;

        if (strncmp(name_p, "125", 3) != 0 && strncmp(name_p, "-125", 4) != 0)
            return Nst_CP_UNKNOWN;

        switch (name_p[0] == '-' ? name_p[4] : name_p[3]) {
        case '0': return Nst_CP_1250;
        case '1': return Nst_CP_1251;
        case '2': return Nst_CP_1252;
        case '3': return Nst_CP_1253;
        case '4': return Nst_CP_1254;
        case '5': return Nst_CP_1255;
        case '6': return Nst_CP_1256;
        case '7': return Nst_CP_1257;
        case '8': return Nst_CP_1258;
        }

        return Nst_CP_UNKNOWN;
    }

    if (strcmp(name_p, "ascii") == 0 || strcmp(name_p, "us-ascii") == 0)
        return Nst_CP_ASCII;

    if (strcmp(name_p, "latin") == 0
        || strcmp(name_p, "latin1") == 0
        || strcmp(name_p, "latin-1") == 0
        || strcmp(name_p, "iso-8859-1") == 0
        || strcmp(name_p, "iso8859-1") == 0
        || strcmp(name_p, "l1") == 0)
    {
        return Nst_CP_ISO8859_1;
    }
    return Nst_CP_UNKNOWN;
}

Nst_CPID Nst_single_byte_cp(Nst_CPID cpid)
{
    if (cpid == Nst_CP_UTF16)
        return Nst_CP_UTF16LE;
    else if (cpid == Nst_CP_UTF32)
        return Nst_CP_UTF32LE;
    else if (cpid == Nst_CP_EXT_UTF16)
        return Nst_CP_EXT_UTF16LE;
    return cpid;
}
