#include <string.h>
#include <wchar.h>
#include "encoding.h"
#include "mem.h"
#include "lib_import.h"
#include "format.h"

Nst_CP nst_cp_ascii = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "ASCII",
    .check_bytes = (Nst_CheckBytesFunc)nst_check_ascii_bytes,
    .to_utf32    = (Nst_ToUTF32Func)nst_ascii_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)nst_ascii_from_utf32,
};

Nst_CP nst_cp_utf8 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8) * 4,
    .mult_min_sz = sizeof(u8),
    .name = "UTF-8",
    .check_bytes = (Nst_CheckBytesFunc)nst_check_utf8_bytes,
    .to_utf32    = (Nst_ToUTF32Func)nst_utf8_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)nst_utf8_from_utf32,
};

Nst_CP nst_cp_utf16 = {
    .ch_size = sizeof(u16),
    .mult_max_sz = sizeof(u16) * 2,
    .mult_min_sz = sizeof(u16),
    .name = "UTF-16",
    .check_bytes = (Nst_CheckBytesFunc)nst_check_utf16_bytes,
    .to_utf32    = (Nst_ToUTF32Func)nst_utf16_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)nst_utf16_from_utf32,
};

Nst_CP nst_cp_utf32 = {
    .ch_size = sizeof(u32),
    .mult_max_sz = sizeof(u32),
    .mult_min_sz = sizeof(u32),
    .name = "UTF-32",
    .check_bytes = (Nst_CheckBytesFunc)nst_check_utf32_bytes,
    .to_utf32    = (Nst_ToUTF32Func)nst_utf32_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)nst_utf32_from_utf32,
};

Nst_CP nst_cp_1252 = {
    .ch_size = sizeof(u8),
    .mult_max_sz = sizeof(u8),
    .mult_min_sz = sizeof(u8),
    .name = "CP1252",
    .check_bytes = (Nst_CheckBytesFunc)nst_check_1252_bytes,
    .to_utf32    = (Nst_ToUTF32Func)nst_1252_to_utf32,
    .from_utf32  = (Nst_FromUTF32Func)nst_1252_from_utf32,
};

i32 nst_check_ascii_bytes(u8 *str, usize len)
{
    if ( *str <= 0x7f )
    {
        return 1;
    }
    return -1;
}

u32 nst_ascii_to_utf32(u8 *str)
{
    return (u32)*str;
}

i32 nst_ascii_from_utf32(u32 ch, u8 *str)
{
    if ( ch > 0x7f )
    {
        return -1;
    }
    *str = (u8)ch;
    return 1;
}

i32 nst_check_utf8_bytes(u8 *str, usize len)
{
    i32 n = 0;

    if ( *str <= 0b01111111 )
    {
        return 1;
    }
    else if ( *str >= 0b11110000 && *str <= 0b11110111 )
    {
        if ( len < 4 )
        {
            return -1;
        }
        n = 3;
    }
    else if ( *str >= 0b11100000 && *str <= 0b11101111 )
    {
        if ( len < 3 )
        {
            return -1;
        }
        n = 2;
    }
    else if ( *str >= 0b11000000 && *str <= 0b11011111 )
    {
        if ( len < 2 )
        {
            return -1;
        }
        n = 1;
    }
    else
    {
        return -1;
    }

    for ( i32 i = 0; i < n; i++ )
    {
        if ( *(++str) < 0b10000000 || *str > 0b10111111 )
        {
            return -1;
        }
    }
    return n + 1;
}

u32 nst_utf8_to_utf32(u8 *str)
{
    i32 len = nst_check_utf8_bytes(str, 4);
    u32 n = 0;
    switch ( len )
    {
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

i32 nst_utf8_from_utf32(u32 ch, u8 *str)
{
    if ( ch <= 0x7f )
    {
        *str = (i8)ch;
        return 1;
    }
    else if ( ch <= 0x7ff )
    {
        *str++ = 0b11000000 | (i8)(ch >> 6);
        *str   = 0b10000000 | (i8)(ch & 0x3f);
        return 2;
    }
    else if ( ch <= 0xffff )
    {
        *str++ = 0b11100000 | (i8)(ch >> 12);
        *str++ = 0b10000000 | (i8)(ch >> 6 & 0x3f);
        *str   = 0b10000000 | (i8)(ch & 0x3f);
        return 3;
    }
    else if ( ch > 0x10ffff )
    {
        return -1;
    }
    *str++ = 0b11110000 | (i8)(ch >> 18);
    *str++ = 0b10000000 | (i8)(ch >> 12 & 0x3f);
    *str++ = 0b10000000 | (i8)(ch >> 6 & 0x3f);
    *str   = 0b10000000 | (i8)(ch & 0x3f);
    return 4;
}

i32 nst_check_utf16_bytes(u16 *str, usize len)
{
    if ( *str <= 0xd7ff )
    {
        return 1;
    }
    else if ( *str < 0xd800 || *str > 0xdbff || len < 2 )
    {
        return -1;
    }
    str++;
    if ( *str < 0xdc00 || *str > 0xdfff )
    {
        return -1;
    }
    return 2;
}

u32 nst_utf16_to_utf32(u16 *str)
{
    u32 n;
    if ( *str <= 0xd7ff )
    {
        n = *str;
    }
    else
    {
        n  = (*str++ & 0x3ff) << 10;
        n += (*str   & 0x3ff);
        n += 0x10000;
    }
    return n;
}

i32 nst_utf16_from_utf32(u32 ch, u16 *str)
{
    if ( ch > 0x10FFFF || (ch >= 0xd800 && ch <= 0xdfff) )
    {
        return -1;
    }
    if ( ch < 0xffff )
    {
        *str = (u16)ch;
        return 1;
    }
    ch -= 0x10000;
    *str++ = 0xd800 + ((ch >> 10) & 0x3ff);
    *str   = 0xdc00 + (ch & 0x3ff);
    return 2;
}

i32 nst_check_utf32_bytes(u32 *str, usize len)
{
    if ( len == 0 )
    {
        return -1;
    }
    return 1;
}

u32 nst_utf32_to_utf32(u32 *str)
{
    return *str;
}

i32 nst_utf32_from_utf32(u32 ch, u32 *str)
{
    *str = ch;
    return 1;
}

i32 nst_check_1252_bytes(u8 *str, usize len)
{
    if ( *str <= 0x7f )
    {
        return 1;
    }

    switch ( *str )
    {
    case 0x81: return -1;
    case 0x8d: return -1;
    case 0x8f: return -1;
    case 0x90: return -1;
    case 0x9d: return -1;
    default:
        return 1;
    }
}

u32 nst_1252_to_utf32(u8 *str)
{
    if ( *str <= 0x7f )
    {
        return *str;
    }

    switch ( *str )
    {
    case 0x80: return 0x20ac;
    case 0x82: return 0x201a;
    case 0x83: return 0x0192;
    case 0x84: return 0x201e;
    case 0x85: return 0x2026;
    case 0x86: return 0x2020;
    case 0x87: return 0x2021;
    case 0x88: return 0x02c6;
    case 0x89: return 0x2030;
    case 0x8a: return 0x0160;
    case 0x8b: return 0x2039;
    case 0x8c: return 0x0152;
    case 0x8e: return 0x017d;
    case 0x91: return 0x2018;
    case 0x92: return 0x2019;
    case 0x93: return 0x201c;
    case 0x94: return 0x201d;
    case 0x95: return 0x2022;
    case 0x96: return 0x2013;
    case 0x97: return 0x2014;
    case 0x98: return 0x02dc;
    case 0x99: return 0x2122;
    case 0x9a: return 0x0161;
    case 0x9b: return 0x203a;
    case 0x9c: return 0x0153;
    case 0x9e: return 0x017e;
    case 0x9f: return 0x0178;
    default:
        return 0;
    }
}

i32 nst_1252_from_utf32(u32 ch, u8 *str)
{
    if ( ch <= 0x7f )
    {
        *str = (u8)ch;
        return 1;
    }

    switch ( ch )
    {
    case 0x20ac: *str = 0x80; return 1;
    case 0x201a: *str = 0x82; return 1;
    case 0x0192: *str = 0x83; return 1;
    case 0x201e: *str = 0x84; return 1;
    case 0x2026: *str = 0x85; return 1;
    case 0x2020: *str = 0x86; return 1;
    case 0x2021: *str = 0x87; return 1;
    case 0x02c6: *str = 0x88; return 1;
    case 0x2030: *str = 0x89; return 1;
    case 0x0160: *str = 0x8a; return 1;
    case 0x2039: *str = 0x8b; return 1;
    case 0x0152: *str = 0x8c; return 1;
    case 0x017d: *str = 0x8e; return 1;
    case 0x2018: *str = 0x91; return 1;
    case 0x2019: *str = 0x92; return 1;
    case 0x201c: *str = 0x93; return 1;
    case 0x201d: *str = 0x94; return 1;
    case 0x2022: *str = 0x95; return 1;
    case 0x2013: *str = 0x96; return 1;
    case 0x2014: *str = 0x97; return 1;
    case 0x02dc: *str = 0x98; return 1;
    case 0x2122: *str = 0x99; return 1;
    case 0x0161: *str = 0x9a; return 1;
    case 0x203a: *str = 0x9b; return 1;
    case 0x0153: *str = 0x9c; return 1;
    case 0x017e: *str = 0x9e; return 1;
    case 0x0178: *str = 0x9f; return 1;
    default:
        return -1;
    }
}

i32 nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len)
{
    if ( nst_check_utf16_bytes(in_str, in_str_len) == -1 )
    {
        return -1;
    }

    u32 n = nst_utf16_to_utf32(in_str);
    return nst_utf8_from_utf32(n, (u8 *)out_str);
}

i32 nst_cp1252_to_utf8(i8 *str, i8 byte)
{
    if ( nst_check_1252_bytes((u8 *)&byte, 1) == -1 )
    {
        return -1;
    }

    u32 n = nst_1252_to_utf32((u8 *)&byte);
    return nst_utf8_from_utf32(n, (u8 *)str);
}

bool nst_translate_cp(Nst_CP *from,
                      Nst_CP *to,
                      void   *from_buf,
                      usize   from_len,
                      void  **to_buf,
                      usize  *to_len,
                      Nst_OpErr *err)
{
    if ( from == NULL || to == NULL )
    {
        NST_SET_RAW_VALUE_ERROR(_NST_EM_INVALID_CPID);
        return false;
    }

    if ( to_len != NULL )
    {
        *to_len = 0;
    }
    *to_buf = NULL;

    Nst_Buffer buf;
    if ( !nst_buffer_init(&buf, from_len * to->mult_min_sz, err) )
    {
        return false;
    }

    isize n = (isize)from_len; // avoids accidental underflow
    for ( ; n > 0; )
    {
        // Decode character
        i32 ch_len = from->check_bytes(from_buf, n);
        if ( ch_len < 0 )
        {
            nst_buffer_destroy(&buf);
            NST_SET_VALUE_ERROR(nst_sprintf(
                _NST_EM_INVALID_ENCODING,
                *(u8 *)from_buf, from->name));
            return false;
        }
        usize ch_size = ch_len * from->ch_size;
        u32 utf32_ch = from->to_utf32(from_buf);
        from_buf = (u8 *)from_buf + ch_size;
        n -= ch_len;

        // Re-encode character
        if ( !nst_buffer_expand_by(&buf, to->mult_max_sz + to->ch_size, err) )
        {
            nst_buffer_destroy(&buf);
            return false;
        }
        ch_len = to->from_utf32(utf32_ch, buf.data + buf.len);
        if ( ch_len == -1 )
        {
            nst_buffer_destroy(&buf);
            NST_SET_VALUE_ERROR(nst_sprintf(
                _NST_EM_INVALID_DECODING,
                (int)utf32_ch, from->name));
        }
        buf.len += ch_len * to->ch_size;
    }
    memset(buf.data + buf.len, 0, to->ch_size);

    *to_buf = buf.data;
    if ( to_len != NULL )
    {
        *to_len = buf.len;
    }
    return true;
}

Nst_CP *nst_cp(Nst_CPID cpid)
{
    switch ( cpid )
    {
    case NST_CP_ASCII:
        return &nst_cp_ascii;
    case NST_CP_UTF8:
        return &nst_cp_utf8;
    case NST_CP_UTF16:
        return &nst_cp_utf16;
    case NST_CP_UTF32:
        return &nst_cp_utf32;
    case NST_CP_1252:
        return &nst_cp_1252;
    default:
        return NULL;
    }
}

wchar_t *nst_char_to_wchar_t(i8 *str, usize len, Nst_OpErr *err)
{
    wchar_t *out_str;
    if ( len == 0 )
    {
        len = strlen(str);
    }

    bool res = nst_translate_cp(
        &nst_cp_utf8, &nst_cp_utf16,
        (void *)str, len, (void **)&out_str, NULL, err);
    if ( res == false )
    {
        return NULL;
    }
    return out_str;
}

i8 *nst_wchar_t_to_char(wchar_t *str, usize len, Nst_OpErr *err)
{
    i8 *out_str;
    if ( len == 0 )
    {
        len = wcslen(str);
    }

    bool res = nst_translate_cp(
        &nst_cp_utf16, &nst_cp_utf8,
        (void *)str, len, (void **)&out_str, NULL, err);
    if ( res == false )
    {
        return NULL;
    }
    return out_str;
}
