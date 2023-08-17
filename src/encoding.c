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

Nst_CP Nst_cp_utf16 = {
    .ch_size = sizeof(u16),
    .mult_max_sz = sizeof(u16) * 2,
    .mult_min_sz = sizeof(u16),
    .name = "UTF-16",
#if Nst_ENDIANNESS == Nst_LITTLE_ENDIAN
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

Nst_CP Nst_cp_utf32 = {
    .ch_size = sizeof(u32),
    .mult_max_sz = sizeof(u32),
    .mult_min_sz = sizeof(u32),
    .name = "UTF-32",
#if Nst_ENDIANNESS == Nst_LITTLE_ENDIAN
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

i32 Nst_check_utf8_bytes(u8 *str, usize len)
{
    i32 n = 0;

    if (*str <= 0b01111111)
        return 1;
    else if (*str >= 0b11110000 && *str <= 0b11110111) {
        if (len < 4)
            return -1;
        n = 3;
    } else if (*str >= 0b11100000 && *str <= 0b11101111) {
        if (len < 3)
            return -1;
        n = 2;
    } else if (*str >= 0b11000000 && *str <= 0b11011111) {
        if (len < 2)
            return -1;
        n = 1;
    } else
        return -1;

    for (i32 i = 0; i < n; i++) {
        if (*(++str) < 0b10000000 || *str > 0b10111111)
            return -1;
    }

    u32 ch = 0;
    switch (n) {
    case 0:
        return (u32)*str;
    case 1:
        ch += (*str++ & 0x1f) << 6;
        ch += *str & 0x3f;
        break;
    case 2:
        ch += (*str++ & 0xf) << 12;
        ch += (*str++ & 0x3f) << 6;
        ch += *str & 0x3f;
        break;
    default:
        ch += (*str++ & 0x7)  << 18;
        ch += (*str++ & 0x3f) << 12;
        ch += (*str++ & 0x3f) << 6;
        ch += *str & 0x3f;
        break;
    }
    if (ch >= 0xd800 && ch <= 0xdfff)
        return -1;
    return n + 1;
}

u32 Nst_utf8_to_utf32(u8 *str)
{
    i32 len = Nst_check_utf8_bytes(str, 4);
    u32 n = 0;
    switch (len) {
    case 1:
        return (u32)*str;
    case 2:
        n += (*str++ & 0x1f) << 6;
        n += *str & 0x3f;
        return n;
    case 3:
        n += (*str++ & 0xf) << 12;
        n += (*str++ & 0x3f) << 6;
        n += *str & 0x3f;
        return n;
    case 4:
        n += (*str++ & 0x7)  << 18;
        n += (*str++ & 0x3f) << 12;
        n += (*str++ & 0x3f) << 6;
        n += *str & 0x3f;
        return n;
    default:
        return -1;
    }
}

i32 Nst_utf8_from_utf32(u32 ch, u8 *str)
{
    if (ch >= 0xd800 && ch <= 0xdfff)
        return -1;

    return Nst_invalid_utf8_from_utf32(ch, str);
}

i32 Nst_invalid_utf8_from_utf32(u32 ch, u8 *str)
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
    if (*str <= 0xd7ff)
        return 1;
    else if (*str > 0xdbff || len < 2)
        return -1;
    str++;
    if (*str < 0xdc00 || *str > 0xdfff)
        return -1;
    return 2;
}

u32 Nst_utf16_to_utf32(u16 *str)
{
    if (*str <= 0xd7ff)
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
    return Nst_check_utf16_bytes(&wch, 2) * 2;
}

u32 Nst_utf16be_to_utf32(u8 *str)
{
    u16 ch1 = ((u16)str[0] << 8) | (u16)str[1];
    if (ch1 <= 0xd7ff)
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
    return Nst_check_utf16_bytes(&wch, 2) * 2;
}

u32 Nst_utf16le_to_utf32(u8 *str)
{
    u16 ch1 = ((u16)str[1] << 8) | (u16)str[0];
    if (ch1 <= 0xd7ff)
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

i32 Nst_check_1250_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    u8 ch = *str;
    if (ch == 0x81 || ch == 0x83 || ch == 0x88 || ch == 0x90 || ch == 0x98)
        return 1;
    return -1;
}

u32 Nst_1250_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80)
        return (u8)ch;

    switch (ch) {
    case 0x80: return 0x20ac; case 0x82: return 0x201a; case 0x84: return 0x201e;
    case 0x85: return 0x2026; case 0x86: return 0x2020; case 0x87: return 0x2021;
    case 0x89: return 0x2030; case 0x8a: return 0x0160; case 0x8b: return 0x2039;
    case 0x8c: return 0x015a; case 0x8d: return 0x0164; case 0x8e: return 0x017d;
    case 0x8f: return 0x0179; case 0x91: return 0x2018; case 0x92: return 0x2019;
    case 0x93: return 0x201c; case 0x94: return 0x201d; case 0x95: return 0x2022;
    case 0x96: return 0x2013; case 0x97: return 0x2014; case 0x99: return 0x2122;
    case 0x9a: return 0x0161; case 0x9b: return 0x203a; case 0x9c: return 0x015b;
    case 0x9d: return 0x0165; case 0x9e: return 0x017e; case 0x9f: return 0x017a;
    case 0xa1: return 0x02c7; case 0xa2: return 0x02d8; case 0xa3: return 0x0141;
    case 0xa5: return 0x0104; case 0xaa: return 0x015e; case 0xaf: return 0x017b;
    case 0xb2: return 0x02db; case 0xb3: return 0x0142; case 0xb9: return 0x0105;
    case 0xba: return 0x015f; case 0xbc: return 0x013d; case 0xbd: return 0x02dd;
    case 0xbe: return 0x013e; case 0xbf: return 0x017c; case 0xc0: return 0x0154;
    case 0xc3: return 0x0102; case 0xc5: return 0x0139; case 0xc6: return 0x0106;
    case 0xc8: return 0x010c; case 0xca: return 0x0118; case 0xcc: return 0x011a;
    case 0xcf: return 0x010e; case 0xd0: return 0x0110; case 0xd1: return 0x0143;
    case 0xd2: return 0x0147; case 0xd5: return 0x0150; case 0xd8: return 0x0158;
    case 0xd9: return 0x016e; case 0xdb: return 0x0170; case 0xde: return 0x0162;
    case 0xe0: return 0x0155; case 0xe3: return 0x0103; case 0xe5: return 0x013a;
    case 0xe6: return 0x0107; case 0xe8: return 0x010d; case 0xea: return 0x0119;
    case 0xec: return 0x011b; case 0xef: return 0x010f; case 0xf0: return 0x0111;
    case 0xf1: return 0x0144; case 0xf2: return 0x0148; case 0xf5: return 0x0151;
    case 0xf8: return 0x0159; case 0xf9: return 0x016f; case 0xfb: return 0x0171;
    case 0xfe: return 0x0163; case 0xff: return 0x02d9;
    default: return (u32)ch;
    }
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
    return *str == 0x89 ? -1 : 1;
}

u32 Nst_1251_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80)
        return (u8)ch;

    switch (ch) {
    case 0x80: return 0x0402; case 0x81: return 0x0403; case 0x82: return 0x201a;
    case 0x83: return 0x0453; case 0x84: return 0x201e; case 0x85: return 0x2026;
    case 0x86: return 0x2020; case 0x87: return 0x2021; case 0x88: return 0x20ac;
    case 0x89: return 0x2030; case 0x8a: return 0x0409; case 0x8b: return 0x2039;
    case 0x8c: return 0x040a; case 0x8d: return 0x040c; case 0x8e: return 0x040b;
    case 0x8f: return 0x040f; case 0x90: return 0x0452; case 0x91: return 0x2018;
    case 0x92: return 0x2019; case 0x93: return 0x201c; case 0x94: return 0x201d;
    case 0x95: return 0x2022; case 0x96: return 0x2013; case 0x97: return 0x2014;
    case 0x99: return 0x2122; case 0x9a: return 0x0459; case 0x9b: return 0x203a;
    case 0x9c: return 0x045a; case 0x9d: return 0x045c; case 0x9e: return 0x045b;
    case 0x9f: return 0x045f; case 0xa1: return 0x040e; case 0xa2: return 0x045e;
    case 0xa3: return 0x0408; case 0xa5: return 0x0490; case 0xa8: return 0x0401;
    case 0xaa: return 0x0404; case 0xaf: return 0x0407; case 0xb2: return 0x0406;
    case 0xb3: return 0x0456; case 0xb4: return 0x0491; case 0xb8: return 0x0451;
    case 0xb9: return 0x2116; case 0xba: return 0x0454; case 0xbc: return 0x0458;
    case 0xbd: return 0x0405; case 0xbe: return 0x0455; case 0xbf: return 0x0457;
    case 0xc0: return 0x0410; case 0xc1: return 0x0411; case 0xc2: return 0x0412;
    case 0xc3: return 0x0413; case 0xc4: return 0x0414; case 0xc5: return 0x0415;
    case 0xc6: return 0x0416; case 0xc7: return 0x0417; case 0xc8: return 0x0418;
    case 0xc9: return 0x0419; case 0xca: return 0x041a; case 0xcb: return 0x041b;
    case 0xcc: return 0x041c; case 0xcd: return 0x041d; case 0xce: return 0x041e;
    case 0xcf: return 0x041f; case 0xd0: return 0x0420; case 0xd1: return 0x0421;
    case 0xd2: return 0x0422; case 0xd3: return 0x0423; case 0xd4: return 0x0424;
    case 0xd5: return 0x0425; case 0xd6: return 0x0426; case 0xd7: return 0x0427;
    case 0xd8: return 0x0428; case 0xd9: return 0x0429; case 0xda: return 0x042a;
    case 0xdb: return 0x042b; case 0xdc: return 0x042c; case 0xdd: return 0x042d;
    case 0xde: return 0x042e; case 0xdf: return 0x042f; case 0xe0: return 0x0430;
    case 0xe1: return 0x0431; case 0xe2: return 0x0432; case 0xe3: return 0x0433;
    case 0xe4: return 0x0434; case 0xe5: return 0x0435; case 0xe6: return 0x0436;
    case 0xe7: return 0x0437; case 0xe8: return 0x0438; case 0xe9: return 0x0439;
    case 0xea: return 0x043a; case 0xeb: return 0x043b; case 0xec: return 0x043c;
    case 0xed: return 0x043d; case 0xee: return 0x043e; case 0xef: return 0x043f;
    case 0xf0: return 0x0440; case 0xf1: return 0x0441; case 0xf2: return 0x0442;
    case 0xf3: return 0x0443; case 0xf4: return 0x0444; case 0xf5: return 0x0445;
    case 0xf6: return 0x0446; case 0xf7: return 0x0447; case 0xf8: return 0x0448;
    case 0xf9: return 0x0449; case 0xfa: return 0x044a; case 0xfb: return 0x044b;
    case 0xfc: return 0x044c; case 0xfd: return 0x044d; case 0xfe: return 0x044e;
    case 0xff: return 0x044f;
    default: return (u32)ch;
    }
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
    case 0x20ac: *str = 0x88; break; case 0x2030: *str = 0x89; break;
    case 0x0409: *str = 0x8a; break; case 0x2039: *str = 0x8b; break;
    case 0x040a: *str = 0x8c; break; case 0x040c: *str = 0x8d; break;
    case 0x040b: *str = 0x8e; break; case 0x040f: *str = 0x8f; break;
    case 0x0452: *str = 0x90; break; case 0x2018: *str = 0x91; break;
    case 0x2019: *str = 0x92; break; case 0x201c: *str = 0x93; break;
    case 0x201d: *str = 0x94; break; case 0x2022: *str = 0x95; break;
    case 0x2013: *str = 0x96; break; case 0x2014: *str = 0x97; break;
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
    u8 ch = *str;
    if (ch == 0x81 || ch == 0x8d || ch == 0x8f || ch == 0x90 || ch == 0x9d)
        return -1;
    return 1;
}

u32 Nst_1252_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80 || ch > 0x9f)
        return (u32)ch;

    switch (ch) {
    case 0x80: return 0x20ac; case 0x82: return 0x201a; case 0x83: return 0x0192;
    case 0x84: return 0x201e; case 0x85: return 0x2026; case 0x86: return 0x2020;
    case 0x87: return 0x2021; case 0x88: return 0x02c6; case 0x89: return 0x2030;
    case 0x8a: return 0x0160; case 0x8b: return 0x2039; case 0x8c: return 0x0152;
    case 0x8e: return 0x017d; case 0x91: return 0x2018; case 0x92: return 0x2019;
    case 0x93: return 0x201c; case 0x94: return 0x201d; case 0x95: return 0x2022;
    case 0x96: return 0x2013; case 0x97: return 0x2014; case 0x98: return 0x02dc;
    case 0x99: return 0x2122; case 0x9a: return 0x0161; case 0x9b: return 0x203a;
    case 0x9c: return 0x0153; case 0x9e: return 0x017e; case 0x9f: return 0x0178;
    }
    return 0;
}

i32 Nst_1252_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80 || (ch > 0x9f && ch < 0x100)) {
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
    case 0x0178: *str = 0x9f; break;
    default: return -1;
    }
    return 1;
}

i32 Nst_check_1253_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    u8 ch = *str;
    if (ch == 0x81 || ch == 0x88 || ch == 0x8a || ch == 0x8c || ch == 0x8d
        || ch == 0x8e || ch == 0x8f || ch == 0x90 || ch == 0x98 || ch == 0x9a
        || ch == 0x9c || ch == 0x9d || ch == 0x9e || ch == 0x9f || ch == 0xaa
        || ch == 0xd2 || ch == 0xff)
    {
        return -1;
    }
    return 1;
}

u32 Nst_1253_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80)
        return (u8)ch;

    switch (ch) {
    case 0x80: return 0x20ac; case 0x82: return 0x201a; case 0x83: return 0x0192;
    case 0x84: return 0x201e; case 0x85: return 0x2026; case 0x86: return 0x2020;
    case 0x87: return 0x2021; case 0x89: return 0x2030; case 0x8b: return 0x2039;
    case 0x91: return 0x2018; case 0x92: return 0x2019; case 0x93: return 0x201c;
    case 0x94: return 0x201d; case 0x95: return 0x2022; case 0x96: return 0x2013;
    case 0x97: return 0x2014; case 0x99: return 0x2122; case 0x9b: return 0x203a;
    case 0xa1: return 0x0385; case 0xa2: return 0x0386; case 0xaf: return 0x2015;
    case 0xb4: return 0x0384; case 0xb8: return 0x0388; case 0xb9: return 0x0389;
    case 0xba: return 0x038a; case 0xbc: return 0x038c; case 0xbe: return 0x038e;
    case 0xbf: return 0x038f; case 0xc0: return 0x0390; case 0xc1: return 0x0391;
    case 0xc2: return 0x0392; case 0xc3: return 0x0393; case 0xc4: return 0x0394;
    case 0xc5: return 0x0395; case 0xc6: return 0x0396; case 0xc7: return 0x0397;
    case 0xc8: return 0x0398; case 0xc9: return 0x0399; case 0xca: return 0x039a;
    case 0xcb: return 0x039b; case 0xcc: return 0x039c; case 0xcd: return 0x039d;
    case 0xce: return 0x039e; case 0xcf: return 0x039f; case 0xd0: return 0x03a0;
    case 0xd1: return 0x03a1; case 0xd3: return 0x03a3; case 0xd4: return 0x03a4;
    case 0xd5: return 0x03a5; case 0xd6: return 0x03a6; case 0xd7: return 0x03a7;
    case 0xd8: return 0x03a8; case 0xd9: return 0x03a9; case 0xda: return 0x03aa;
    case 0xdb: return 0x03ab; case 0xdc: return 0x03ac; case 0xdd: return 0x03ad;
    case 0xde: return 0x03ae; case 0xdf: return 0x03af; case 0xe0: return 0x03b0;
    case 0xe1: return 0x03b1; case 0xe2: return 0x03b2; case 0xe3: return 0x03b3;
    case 0xe4: return 0x03b4; case 0xe5: return 0x03b5; case 0xe6: return 0x03b6;
    case 0xe7: return 0x03b7; case 0xe8: return 0x03b8; case 0xe9: return 0x03b9;
    case 0xea: return 0x03ba; case 0xeb: return 0x03bb; case 0xec: return 0x03bc;
    case 0xed: return 0x03bd; case 0xee: return 0x03be; case 0xef: return 0x03bf;
    case 0xf0: return 0x03c0; case 0xf1: return 0x03c1; case 0xf2: return 0x03c2;
    case 0xf3: return 0x03c3; case 0xf4: return 0x03c4; case 0xf5: return 0x03c5;
    case 0xf6: return 0x03c6; case 0xf7: return 0x03c7; case 0xf8: return 0x03c8;
    case 0xf9: return 0x03c9; case 0xfa: return 0x03ca; case 0xfb: return 0x03cb;
    case 0xfc: return 0x03cc; case 0xfd: return 0x03cd; case 0xfe: return 0x03ce;
    default: return (u32)ch;
    }
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
    u8 ch = *str;
    if (ch == 0x81 || ch == 0x8d || ch == 0x8e || ch == 0x8f || ch == 0x90
        || ch == 0x9d || ch == 0x9e)
    {
        return -1;
    }
    return 1;
}

u32 Nst_1254_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80)
        return (u8)ch;

    switch (ch) {
    case 0x80: return 0x20ac; case 0x82: return 0x201a; case 0x83: return 0x0192;
    case 0x84: return 0x201e; case 0x85: return 0x2026; case 0x86: return 0x2020;
    case 0x87: return 0x2021; case 0x88: return 0x02c6; case 0x89: return 0x2030;
    case 0x8a: return 0x0160; case 0x8b: return 0x2039; case 0x8c: return 0x0152;
    case 0x91: return 0x2018; case 0x92: return 0x2019; case 0x93: return 0x201c;
    case 0x94: return 0x201d; case 0x95: return 0x2022; case 0x96: return 0x2013;
    case 0x97: return 0x2014; case 0x98: return 0x02dc; case 0x99: return 0x2122;
    case 0x9a: return 0x0161; case 0x9b: return 0x203a; case 0x9c: return 0x0153;
    case 0x9f: return 0x0178; case 0xd0: return 0x011e; case 0xdd: return 0x0130;
    case 0xde: return 0x015e; case 0xf0: return 0x011f; case 0xfd: return 0x0131;
    case 0xfe: return 0x015f;
    default: return (u32)ch;
    }
}

i32 Nst_1254_from_utf32(u32 ch, u8 *str)
{
    if (ch < 0x80) {
        *str = (u8)ch;
        return 1;
    }
    if (ch > 0x9f && ch < 0x100 && ch != 0xd0 && ch != 0xdd && ch != 0xde
        && ch != 0xf0 && ch != 0xfd && ch != 0xfe)
    {
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
    case 0x0178: *str = 0x9f; break; case 0x011e: *str = 0xd0; break;
    case 0x0130: *str = 0xdd; break; case 0x015e: *str = 0xde; break;
    case 0x011f: *str = 0xf0; break; case 0x0131: *str = 0xfd; break;
    case 0x015f: *str = 0xfe; break;
    default: return -1;
    }

    return 1;
}
i32 Nst_check_1255_bytes(u8 *str, usize len)
{
    Nst_UNUSED(len);
    u8 ch = *str;
    if (ch == 0x81 || ch == 0x8a || ch == 0x8c || ch == 0x8d || ch == 0x8e
        || ch == 0x8f || ch == 0x90 || ch == 0x9a || ch == 0x9c || ch == 0x9d
        || ch == 0x9e || ch == 0x9f || ch == 0xca || ch == 0xd9 || ch == 0xda
        || ch == 0xdb || ch == 0xdc || ch == 0xdd || ch == 0xde || ch == 0xdf
        || ch == 0xfb || ch == 0xfc || ch == 0xff)
    {
        return -1;
    }
    return 1;
}

u32 Nst_1255_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80)
        return (u8)ch;

    switch (ch) {
    case 0x80: return 0x20ac; case 0x82: return 0x201a; case 0x83: return 0x0192;
    case 0x84: return 0x201e; case 0x85: return 0x2026; case 0x86: return 0x2020;
    case 0x87: return 0x2021; case 0x88: return 0x02c6; case 0x89: return 0x2030;
    case 0x8b: return 0x2039; case 0x91: return 0x2018; case 0x92: return 0x2019;
    case 0x93: return 0x201c; case 0x94: return 0x201d; case 0x95: return 0x2022;
    case 0x96: return 0x2013; case 0x97: return 0x2014; case 0x98: return 0x02dc;
    case 0x99: return 0x2122; case 0x9b: return 0x203a; case 0xa4: return 0x20aa;
    case 0xaa: return 0x00d7; case 0xba: return 0x00f7; case 0xc0: return 0x05b0;
    case 0xc1: return 0x05b1; case 0xc2: return 0x05b2; case 0xc3: return 0x05b3;
    case 0xc4: return 0x05b4; case 0xc5: return 0x05b5; case 0xc6: return 0x05b6;
    case 0xc7: return 0x05b7; case 0xc8: return 0x05b8; case 0xc9: return 0x05b9;
    case 0xcb: return 0x05bb; case 0xcc: return 0x05bc; case 0xcd: return 0x05bd;
    case 0xce: return 0x05be; case 0xcf: return 0x05bf; case 0xd0: return 0x05c0;
    case 0xd1: return 0x05c1; case 0xd2: return 0x05c2; case 0xd3: return 0x05c3;
    case 0xd4: return 0x05f0; case 0xd5: return 0x05f1; case 0xd6: return 0x05f2;
    case 0xd7: return 0x05f3; case 0xd8: return 0x05f4; case 0xe0: return 0x05d0;
    case 0xe1: return 0x05d1; case 0xe2: return 0x05d2; case 0xe3: return 0x05d3;
    case 0xe4: return 0x05d4; case 0xe5: return 0x05d5; case 0xe6: return 0x05d6;
    case 0xe7: return 0x05d7; case 0xe8: return 0x05d8; case 0xe9: return 0x05d9;
    case 0xea: return 0x05da; case 0xeb: return 0x05db; case 0xec: return 0x05dc;
    case 0xed: return 0x05dd; case 0xee: return 0x05de; case 0xef: return 0x05df;
    case 0xf0: return 0x05e0; case 0xf1: return 0x05e1; case 0xf2: return 0x05e2;
    case 0xf3: return 0x05e3; case 0xf4: return 0x05e4; case 0xf5: return 0x05e5;
    case 0xf6: return 0x05e6; case 0xf7: return 0x05e7; case 0xf8: return 0x05e8;
    case 0xf9: return 0x05e9; case 0xfa: return 0x05ea; case 0xfd: return 0x200e;
    case 0xfe: return 0x200f;
    default: return (u32)ch;
    }
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
    Nst_UNUSED(str);
    Nst_UNUSED(len);
    return 1;
}

u32 Nst_1256_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80)
        return (u8)ch;

    switch (ch) {
    case 0x80: return 0x20ac; case 0x81: return 0x067e; case 0x82: return 0x201a;
    case 0x83: return 0x0192; case 0x84: return 0x201e; case 0x85: return 0x2026;
    case 0x86: return 0x2020; case 0x87: return 0x2021; case 0x88: return 0x02c6;
    case 0x89: return 0x2030; case 0x8a: return 0x0679; case 0x8b: return 0x2039;
    case 0x8c: return 0x0152; case 0x8d: return 0x0686; case 0x8e: return 0x0698;
    case 0x8f: return 0x0688; case 0x90: return 0x06af; case 0x91: return 0x2018;
    case 0x92: return 0x2019; case 0x93: return 0x201c; case 0x94: return 0x201d;
    case 0x95: return 0x2022; case 0x96: return 0x2013; case 0x97: return 0x2014;
    case 0x98: return 0x06a9; case 0x99: return 0x2122; case 0x9a: return 0x0691;
    case 0x9b: return 0x203a; case 0x9c: return 0x0153; case 0x9d: return 0x200c;
    case 0x9e: return 0x200d; case 0x9f: return 0x06ba; case 0xa1: return 0x060c;
    case 0xaa: return 0x06be; case 0xba: return 0x061b; case 0xbf: return 0x061f;
    case 0xc0: return 0x06c1; case 0xc1: return 0x0621; case 0xc2: return 0x0622;
    case 0xc3: return 0x0623; case 0xc4: return 0x0624; case 0xc5: return 0x0625;
    case 0xc6: return 0x0626; case 0xc7: return 0x0627; case 0xc8: return 0x0628;
    case 0xc9: return 0x0629; case 0xca: return 0x062a; case 0xcb: return 0x062b;
    case 0xcc: return 0x062c; case 0xcd: return 0x062d; case 0xce: return 0x062e;
    case 0xcf: return 0x062f; case 0xd0: return 0x0630; case 0xd1: return 0x0631;
    case 0xd2: return 0x0632; case 0xd3: return 0x0633; case 0xd4: return 0x0634;
    case 0xd5: return 0x0635; case 0xd6: return 0x0636; case 0xd8: return 0x0637;
    case 0xd9: return 0x0638; case 0xda: return 0x0639; case 0xdb: return 0x063a;
    case 0xdc: return 0x0640; case 0xdd: return 0x0641; case 0xde: return 0x0642;
    case 0xdf: return 0x0643; case 0xe1: return 0x0644; case 0xe3: return 0x0645;
    case 0xe4: return 0x0646; case 0xe5: return 0x0647; case 0xe6: return 0x0648;
    case 0xec: return 0x0649; case 0xed: return 0x064a; case 0xf0: return 0x064b;
    case 0xf1: return 0x064c; case 0xf2: return 0x064d; case 0xf3: return 0x064e;
    case 0xf5: return 0x064f; case 0xf6: return 0x0650; case 0xf8: return 0x0651;
    case 0xfa: return 0x0652; case 0xfd: return 0x200e; case 0xfe: return 0x200f;
    case 0xff: return 0x06d2;
    default: return (u32)ch;
    }
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
    u8 ch = *str;
    if (ch == 0x81 || ch == 0x83 || ch == 0x88 || ch == 0x8a || ch == 0x8c
        || ch == 0x90 || ch == 0x98 || ch == 0x9a || ch == 0x9c || ch == 0x9f
        || ch == 0xa1 || ch == 0xa5)
    {
        return -1;
    }
    return 1;
}

u32 Nst_1257_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80)
        return (u8)ch;
    switch (ch) {
    case 0x80: return 0x20ac; case 0x82: return 0x201a; case 0x84: return 0x201e;
    case 0x85: return 0x2026; case 0x86: return 0x2020; case 0x87: return 0x2021;
    case 0x89: return 0x2030; case 0x8b: return 0x2039; case 0x8d: return 0x00a8;
    case 0x8e: return 0x02c7; case 0x8f: return 0x00b8; case 0x91: return 0x2018;
    case 0x92: return 0x2019; case 0x93: return 0x201c; case 0x94: return 0x201d;
    case 0x95: return 0x2022; case 0x96: return 0x2013; case 0x97: return 0x2014;
    case 0x99: return 0x2122; case 0x9b: return 0x203a; case 0x9d: return 0x00af;
    case 0x9e: return 0x02db; case 0xa8: return 0x00d8; case 0xaa: return 0x0156;
    case 0xaf: return 0x00c6; case 0xb8: return 0x00f8; case 0xba: return 0x0157;
    case 0xbf: return 0x00e6; case 0xc0: return 0x0104; case 0xc1: return 0x012e;
    case 0xc2: return 0x0100; case 0xc3: return 0x0106; case 0xc6: return 0x0118;
    case 0xc7: return 0x0112; case 0xc8: return 0x010c; case 0xca: return 0x0179;
    case 0xcb: return 0x0116; case 0xcc: return 0x0122; case 0xcd: return 0x0136;
    case 0xce: return 0x012a; case 0xcf: return 0x013b; case 0xd0: return 0x0160;
    case 0xd1: return 0x0143; case 0xd2: return 0x0145; case 0xd4: return 0x014c;
    case 0xd8: return 0x0172; case 0xd9: return 0x0141; case 0xda: return 0x015a;
    case 0xdb: return 0x016a; case 0xdd: return 0x017b; case 0xde: return 0x017d;
    case 0xe0: return 0x0105; case 0xe1: return 0x012f; case 0xe2: return 0x0101;
    case 0xe3: return 0x0107; case 0xe6: return 0x0119; case 0xe7: return 0x0113;
    case 0xe8: return 0x010d; case 0xea: return 0x017a; case 0xeb: return 0x0117;
    case 0xec: return 0x0123; case 0xed: return 0x0137; case 0xee: return 0x012b;
    case 0xef: return 0x013c; case 0xf0: return 0x0161; case 0xf1: return 0x0144;
    case 0xf2: return 0x0146; case 0xf4: return 0x014d; case 0xf8: return 0x0173;
    case 0xf9: return 0x0142; case 0xfa: return 0x015b; case 0xfb: return 0x016b;
    case 0xfd: return 0x017c; case 0xfe: return 0x017e; case 0xff: return 0x02d9;
    default: return (u32)ch;
    }
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
    u8 ch = *str;
    if (ch == 0x81 || ch == 0x8a || ch == 0x8d || ch == 0x8e || ch == 0x8f
        || ch == 0x90 || ch == 0x9a || ch == 0x9d || ch == 0x9e)
    {
        return -1;
    }
    return 1;
}

u32 Nst_1258_to_utf32(u8 *str)
{
    u8 ch = *str;
    if (ch < 0x80)
        return (u8)ch;

    switch (ch) {
    case 0x80: return 0x20ac; case 0x82: return 0x201a; case 0x83: return 0x0192;
    case 0x84: return 0x201e; case 0x85: return 0x2026; case 0x86: return 0x2020;
    case 0x87: return 0x2021; case 0x88: return 0x02c6; case 0x89: return 0x2030;
    case 0x8b: return 0x2039; case 0x8c: return 0x0152; case 0x91: return 0x2018;
    case 0x92: return 0x2019; case 0x93: return 0x201c; case 0x94: return 0x201d;
    case 0x95: return 0x2022; case 0x96: return 0x2013; case 0x97: return 0x2014;
    case 0x98: return 0x02dc; case 0x99: return 0x2122; case 0x9b: return 0x203a;
    case 0x9c: return 0x0153; case 0x9f: return 0x0178; case 0xc3: return 0x0102;
    case 0xcc: return 0x0300; case 0xd0: return 0x0110; case 0xd2: return 0x0309;
    case 0xd5: return 0x01a0; case 0xdd: return 0x01af; case 0xde: return 0x0303;
    case 0xe3: return 0x0103; case 0xec: return 0x0301; case 0xf0: return 0x0111;
    case 0xf2: return 0x0323; case 0xf5: return 0x01a1; case 0xfd: return 0x01b0;
    case 0xfe: return 0x20ab;
    default: return (u32)ch;
    }
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
    for (usize i = 0; i < str_len; i++) {
        i32 ch_len = cp->check_bytes(str, str_len - i);
        if (ch_len < 0)
            return i;
        usize ch_size = ch_len * cp->ch_size;
        str = (u8 *)str + ch_size;
        i += ch_len - 1;
    }
    return -1;
}

Nst_CP *Nst_cp(Nst_CPID cpid)
{
    switch (cpid) {
    case Nst_CP_ASCII:   return &Nst_cp_ascii;
    case Nst_CP_UTF8:    return &Nst_cp_utf8;
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
    case 1200: return Nst_CP_UTF16LE;
    case 1201: return Nst_CP_UTF16BE;
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
        &Nst_cp_utf8,
        &Nst_cp_utf16,
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
        &Nst_cp_utf16,
        &Nst_cp_utf8,
        (void *)str, len,
        (void **)&out_str, NULL);
    if (res == false)
        return NULL;
    return out_str;
}

bool Nst_is_valid_cp(u32 cp)
{
    return cp <= 0x10ffff
        && (cp < 0xd800 || cp > 0xdfff)
        && (cp < 0xfdd0 || cp > 0xfdef)
        && ((cp & 0xfff0) != 0xfff0 || (cp & 0xf) < 0xe);
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

#if Nst_ENDIANNESS == Nst_LITTLE_ENDIAN
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

    for (usize i = 0; i < name_len; i++) {
        name_cpy[i] = tolower((u8)name[i]);
        if (name_cpy[i] == '_')
            name_cpy[i] = '-';
    }
    name_cpy[name_len] = 0;

    if (name_len > 3 && strncmp(name_cpy, "utf", 3) == 0) {
        name = name_cpy + 3;
        if (strcmp(name, "8") == 0 || strcmp(name, "-8") == 0)
            return Nst_CP_UTF8;
        if (strcmp(name, "16") == 0 || strcmp(name, "-16") == 0
            || strcmp(name, "16le") == 0 || strcmp(name, "-16le") == 0)
        {
            return Nst_CP_UTF16LE;
        }
        if (strcmp(name, "16be") == 0 || strcmp(name, "-16be") == 0)
            return Nst_CP_UTF16BE;
        if (strcmp(name, "32") == 0 || strcmp(name, "-32") == 0
            || strcmp(name, "32le") == 0 || strcmp(name, "-32le") == 0)
        {
            return Nst_CP_UTF32LE;
        }
        if (strcmp(name, "32be") == 0 || strcmp(name, "-32be") == 0)
            return Nst_CP_UTF32BE;
        return Nst_CP_UNKNOWN;
    }

    if ((name_len > 2 && strncmp(name_cpy, "cp", 2) == 0)
        || (name_len > 7 && strncmp(name_cpy, "windows", 7) == 0))
    {
        if (name_cpy[0] == 'c')
            name = name_cpy + 2;
        else
            name = name_cpy + 7;

        if (strncmp(name, "125", 3) != 0 && strncmp(name, "-125", 4) != 0)
            return Nst_CP_UNKNOWN;

        switch (name[0] == '-' ? name[4] : name[3]) {
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

    if (strcmp(name_cpy, "ascii") == 0 || strcmp(name_cpy, "us-ascii") == 0)
        return Nst_CP_ASCII;

    if (strcmp(name_cpy, "latin") == 0
        || strcmp(name_cpy, "latin1") == 0
        || strcmp(name_cpy, "latin-1") == 0
        || strcmp(name_cpy, "iso-8859-1") == 0
        || strcmp(name_cpy, "iso8859-1") == 0
        || strcmp(name_cpy, "l1") == 0)
    {
        return Nst_CP_ISO8859_1;
    }
    return Nst_CP_UNKNOWN;
}
