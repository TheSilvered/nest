#include "encoding.h"

i32 nst_check_utf8_bytes(u8 *byte, usize len)
{
    i32 n = 0;

    if ( *byte <= 0b01111111 )
    {
        return 1;
    }
    else if ( *byte >= 0b11110000 && *byte <= 0b11110111 )
    {
        if ( len < 4 )
        {
            return -1;
        }
        n = 3;
    }
    else if ( *byte >= 0b11100000 && *byte <= 0b11101111 )
    {
        if ( len < 3 )
        {
            return -1;
        }
        n = 2;
    }
    else if ( *byte >= 0b11000000 && *byte <= 0b11011111 )
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
        if ( *(++byte) < 0b10000000 || *byte > 0b10111111 )
        {
            return -1;
        }
    }
    return n + 1;
}

i32 nst_check_utf16_bytes(u16 *byte, usize len)
{
    if ( *byte <= 0xd7ff )
    {
        return 1;
    }
    else if ( *byte < 0xd800 || *byte > 0xdbff || len < 2 )
    {
        return -1;
    }
    byte++;
    if ( *byte < 0xdc00 || *byte > 0xdfff )
    {
        return -1;
    }
    return 2;
}

i32 nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len)
{
    if ( nst_check_utf16_bytes(in_str, in_str_len) == -1 )
    {
        return -1;
    }

    u32 n;
    if ( *in_str <= 0xd7ff )
    {
        n = *in_str;
    }
    else
    {
        n  = (*in_str++ & 0x3ff) + 0xd800;
        n += (*in_str   & 0x3ff);
    }

    if ( n <= 0x7f )
    {
        *out_str = (i8)n;
        return 1;
    }
    else if ( n <= 0x7ff )
    {
        *out_str++ = 0b11000000 | (i8)(n >> 6);
        *out_str   = 0b10000000 | (i8)(n & 0x3f);
        return 2;
    }
    else if ( n <= 0xffff )
    {
        *out_str++ = 0b11100000 | (i8)(n >> 12);
        *out_str++ = 0b10000000 | (i8)(n >> 6 & 0x3f);
        *out_str   = 0b10000000 | (i8)(n & 0x3f);
        return 3;
    }
    else if ( n > 0x10ffff )
    {
        return -1;
    }
    *out_str++ = 0b11110000 | (i8)(n >> 18);
    *out_str++ = 0b10000000 | (i8)(n >> 12 & 0x3f);
    *out_str++ = 0b10000000 | (i8)(n >> 6 & 0x3f);
    *out_str   = 0b10000000 | (i8)(n & 0x3f);
    return 4;
}

i32 nst_cp1252_to_utf8(i8 *str, i8 byte)
{
    u8 b = (u8)byte;
    if ( b <= 0x7f )
    {
        *str = byte;
        return 1;
    }

    u8 b1, b2, b3;
    switch ( b )
    {
    case 0x80: b1 = 0xe2; b2 = 0x82; b3 = 0xac; break;
    case 0x81: return -1;
    case 0x82: b1 = 0xe2; b2 = 0x80; b3 = 0x9a; break;
    case 0x83: b1 = 0xc6; b2 = 0x92; b3 = 0x00; break;
    case 0x84: b1 = 0xe2; b2 = 0x80; b3 = 0x9e; break;
    case 0x85: b1 = 0xe2; b2 = 0x80; b3 = 0xa6; break;
    case 0x86: b1 = 0xe2; b2 = 0x80; b3 = 0xa0; break;
    case 0x87: b1 = 0xe2; b2 = 0x80; b3 = 0xa1; break;
    case 0x88: b1 = 0xcb; b2 = 0x86; b3 = 0x00; break;
    case 0x89: b1 = 0xe2; b2 = 0x80; b3 = 0xb0; break;
    case 0x8a: b1 = 0xc5; b2 = 0xa0; b3 = 0x00; break;
    case 0x8b: b1 = 0xe2; b2 = 0x80; b3 = 0xb9; break;
    case 0x8c: b1 = 0xc5; b2 = 0x92; b3 = 0x00; break;
    case 0x8d: return -1;
    case 0x8e: b1 = 0xc5; b2 = 0xbd; b3 = 0x00; break;
    case 0x8f: return -1;
    case 0x90: return -1;
    case 0x91: b1 = 0xe2; b2 = 0x80; b3 = 0x98; break;
    case 0x92: b1 = 0xe2; b2 = 0x80; b3 = 0x99; break;
    case 0x93: b1 = 0xe2; b2 = 0x80; b3 = 0x9c; break;
    case 0x94: b1 = 0xe2; b2 = 0x80; b3 = 0x9d; break;
    case 0x95: b1 = 0xe2; b2 = 0x80; b3 = 0xa2; break;
    case 0x96: b1 = 0xe2; b2 = 0x80; b3 = 0x93; break;
    case 0x97: b1 = 0xe2; b2 = 0x80; b3 = 0x94; break;
    case 0x98: b1 = 0xcb; b2 = 0x9c; b3 = 0x00; break;
    case 0x99: b1 = 0xe2; b2 = 0x84; b3 = 0xa2; break;
    case 0x9a: b1 = 0xc5; b2 = 0xa1; b3 = 0x00; break;
    case 0x9b: b1 = 0xe2; b2 = 0x80; b3 = 0xba; break;
    case 0x9c: b1 = 0xc5; b2 = 0x93; b3 = 0x00; break;
    case 0x9d: return -1;
    case 0x9e: b1 = 0xc5; b2 = 0xbe; b3 = 0x00; break;
    case 0x9f: b1 = 0xc5; b2 = 0xb8; b3 = 0x00; break;
    default:
        *str++ = 0b11000000 + (b >> 6);
        *str   = 0b10000000 + (b & 0x3f);
        return 2;
    }

    *str++ = b1;
    *str++ = b2;
    if ( b3 == 0 )
    {
        return 2;
    }
    *str = b3;
    return 3;
}
