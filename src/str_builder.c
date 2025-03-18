#include <string.h>
#include "nest.h"

#define _SB_MIN_CAP 127

bool Nst_sb_init(Nst_StrBuilder *sb, usize reserve)
{
    if (reserve < _SB_MIN_CAP)
        reserve = _SB_MIN_CAP;
    u8 *value = Nst_malloc_c(reserve + 1, u8);
    if (value == NULL) {
        sb->value = NULL;
        sb->cap = 0;
        sb->len = 0;
        return false;
    }
    value[0] = '\0';
    sb->value = value;
    sb->cap = reserve;
    sb->len = 0;
    return true;
}

void Nst_sb_destroy(Nst_StrBuilder *sb)
{
    if (sb->value != NULL)
        Nst_free(sb->value);
    sb->value = NULL;
    sb->len = 0;
    sb->cap = 0;
}

bool Nst_sb_reserve(Nst_StrBuilder *sb, usize amount)
{
    usize new_cap = sb->len + amount;
    if (new_cap <= sb->cap)
        return true;
    new_cap += new_cap >> 1;
    u8 *new_data = Nst_realloc_c(sb->value, new_cap + 1, u8, 0);
    if (new_data == NULL)
        return false;
    sb->value = new_data;
    sb->cap = new_cap;
    return true;
}

bool Nst_sb_push(Nst_StrBuilder *sb, u8 *chars, usize count)
{
    if (!Nst_sb_reserve(sb, count))
        return false;
    memcpy(sb->value + sb->len, chars, count);
    sb->len += count;
    sb->value[sb->len] = '\0';
    return true;
}

bool Nst_sb_push_sv(Nst_StrBuilder *sb, Nst_StrView sv)
{
    if (!Nst_sb_reserve(sb, sv.len))
        return false;
    memcpy(sb->value + sb->len, sv.value, sv.len);
    sb->len += sv.len;
    sb->value[sb->len] = '\0';
    return true;
}

bool Nst_sb_push_str(Nst_StrBuilder *sb, Nst_Obj *str)
{
    usize str_len = Nst_str_len(str);
    if (!Nst_sb_reserve(sb, str_len))
        return false;
    memcpy(sb->value + sb->len, Nst_str_value(str), str_len);
    sb->len += str_len;
    sb->value[sb->len] = '\0';
    return true;
}

bool Nst_sb_push_c(Nst_StrBuilder *sb, const char *s)
{
    usize str_len = strlen(s);
    if (!Nst_sb_reserve(sb, str_len))
        return false;
    memcpy(sb->value + sb->len, s, str_len);
    sb->len += str_len;
    sb->value[sb->len] = '\0';
    return true;
}

bool Nst_sb_push_cps(Nst_StrBuilder *sb, u32 *cps, usize count)
{
    for (usize i = 0; i < count; i++) {
        u32 cp = cps[i];
        if (!Nst_sb_reserve(sb, Nst_ENCODING_MULTIBYTE_MAX_SIZE))
            return false;
        usize expanded_size = Nst_ext_utf8_from_utf32(
            cp,
            (u8 *)sb->value + sb->len);
        sb->len += expanded_size;
        sb->value[sb->len] = '\0';
    }
    return true;
}

bool Nst_sb_push_char(Nst_StrBuilder *sb, char ch)
{
    if (!Nst_sb_reserve(sb, 1))
        return false;
    sb->value[sb->len++] = ch;
    sb->value[sb->len] = '\0';
    return true;
}

Nst_Obj *Nst_str_from_sb(Nst_StrBuilder *sb)
{
    sb->value = Nst_realloc_c(sb->value, sb->len + 1, u8, sb->cap + 1);

    Nst_Obj *str = Nst_str_new(sb->value, sb->len, true);
    if (str == NULL)
        return NULL;

    sb->value = NULL;
    sb->len = 0;
    sb->cap = 0;

    return str;
}

Nst_StrView Nst_sv_from_sb(Nst_StrBuilder *sb)
{
    Nst_StrView sv = {
        .value = sb->value,
        .len = sb->len
    };
    return sv;
}
