/**
 * @file encoding.h
 *
 * @brief Functions to manage various encodings
 *
 * @author TheSilvered
 */

#ifndef ENCODING_H
#define ENCODING_H

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/** The supported encodings in Nest
 *
 * @brief Nst_CP_UNKNOWN is -1, Nst_CP_LATIN1 and Nst_CP_ISO8859_1 are
 * equivalent
 */
NstEXP typedef enum _Nst_CPID {
    Nst_CP_UNKNOWN = -1,
    Nst_CP_ASCII,
    Nst_CP_UTF8,
    Nst_CP_UTF16,
    Nst_CP_UTF16BE,
    Nst_CP_UTF16LE,
    Nst_CP_UTF32,
    Nst_CP_UTF32BE,
    Nst_CP_UTF32LE,
    Nst_CP_1250,
    Nst_CP_1251,
    Nst_CP_1252,
    Nst_CP_1253,
    Nst_CP_1254,
    Nst_CP_1255,
    Nst_CP_1256,
    Nst_CP_1257,
    Nst_CP_1258,
    Nst_CP_LATIN1,
    Nst_CP_ISO8859_1 = Nst_CP_LATIN1
} Nst_CPID;

/**
 * @brief The signature of a function that checks the length of the first
 * character in a string of a certain encoding
 */
NstEXP typedef i32 (*Nst_CheckBytesFunc)(void *str, usize len);

/**
* @brief The signature of a function that returns the code point of the first
* character in a string of a certain encoding, expecting a valid sequence of
* bytes
*/
NstEXP typedef u32 (*Nst_ToUTF32Func)(void *str);

/**
 * @brief The signature of a function that encodesa code point in a certain
 * encoding and writes the output to a buffer
 */
NstEXP typedef i32 (*Nst_FromUTF32Func)(u32 ch, void *buf);

/** The structure that represents an encoding
 *
 * @param ch_size: the size of one unit in bytes (e.g. is 1 in UTF-8 but 2 in
 * UTF-16)
 * @param mult_max_sz: the size in bytes of the longest character
 * @param mult_min_sz: the size in bytes of the shortest character (usually the
 * same as ch_size)
 * @param name: the name of the encoding displayed in errors
 * @param check_bytes: the Nst_CheckBytesFunc function of the encoding
 * @param to_utf32: the Nst_ToUTF32Func function of the encoding
 * @param from_utf32: the Nst_FromUTF32Func function of the encoding
 */
NstEXP typedef struct _Nst_CP {
    usize ch_size;
    usize mult_max_sz;
    usize mult_min_sz;
    const i8 *name;
    Nst_CheckBytesFunc check_bytes;
    Nst_ToUTF32Func to_utf32;
    Nst_FromUTF32Func from_utf32;
} Nst_CP;

extern Nst_CP Nst_cp_ascii;
extern Nst_CP Nst_cp_utf8;
extern Nst_CP Nst_cp_utf16;
extern Nst_CP Nst_cp_utf16be;
extern Nst_CP Nst_cp_utf16le;
extern Nst_CP Nst_cp_utf32;
extern Nst_CP Nst_cp_utf32be;
extern Nst_CP Nst_cp_utf32le;
extern Nst_CP Nst_cp_1250;
extern Nst_CP Nst_cp_1251;
extern Nst_CP Nst_cp_1252;
extern Nst_CP Nst_cp_1253;
extern Nst_CP Nst_cp_1254;
extern Nst_CP Nst_cp_1255;
extern Nst_CP Nst_cp_1256;
extern Nst_CP Nst_cp_1257;
extern Nst_CP Nst_cp_1258;
extern Nst_CP Nst_cp_iso8859_1;

/* Nst_CheckBytesFunc for ASCII */
NstEXP i32 NstC Nst_check_ascii_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for ASCII */
NstEXP u32 NstC Nst_ascii_to_utf32(u8 *str);
/* Nst_FromUTF32Func for ASCII */
NstEXP i32 NstC Nst_ascii_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for UTF-8 */
NstEXP i32 NstC Nst_check_utf8_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for UTF-8 */
NstEXP u32 NstC Nst_utf8_to_utf32(u8 *str);
/* Nst_FromUTF32Func for UTF-8 */
NstEXP i32 NstC Nst_utf8_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for UTF-16 */
NstEXP i32 NstC Nst_check_utf16_bytes(u16 *str, usize len);
/* Nst_ToUTF32Func for UTF-16 */
NstEXP u32 NstC Nst_utf16_to_utf32(u16 *str);
/* Nst_FromUTF32Func for UTF-16 */
NstEXP i32 NstC Nst_utf16_from_utf32(u32 ch, u16 *str);

/* Nst_CheckBytesFunc for UTF-16BE */
NstEXP i32 NstC Nst_check_utf16be_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for UTF-16BE */
NstEXP u32 NstC Nst_utf16be_to_utf32(u8 *str);
/* Nst_FromUTF32Func for UTF-16BE */
NstEXP i32 NstC Nst_utf16be_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for UTF-16LE */
NstEXP i32 NstC Nst_check_utf16le_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for UTF-16LE */
NstEXP u32 NstC Nst_utf16le_to_utf32(u8 *str);
/* Nst_FromUTF32Func for UTF-16LE */
NstEXP i32 NstC Nst_utf16le_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for UTF-32 */
NstEXP i32 NstC Nst_check_utf32_bytes(u32 *str, usize len);
/* Nst_ToUTF32Func for UTF-32 */
NstEXP u32 NstC Nst_utf32_to_utf32(u32 *str);
/* Nst_FromUTF32Func for UTF-32 */
NstEXP i32 NstC Nst_utf32_from_utf32(u32 ch, u32 *str);

/* Nst_CheckBytesFunc for UTF-32BE */
NstEXP i32 NstC Nst_check_utf32be_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for UTF-32BE */
NstEXP u32 NstC Nst_utf32be_to_utf32(u8 *str);
/* Nst_FromUTF32Func for UTF-32BE */
NstEXP i32 NstC Nst_utf32be_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for UTF-32LE */
NstEXP i32 NstC Nst_check_utf32le_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for UTF-32LE */
NstEXP u32 NstC Nst_utf32le_to_utf32(u8 *str);
/* Nst_FromUTF32Func for UTF-32LE */
NstEXP i32 NstC Nst_utf32le_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1250 */
NstEXP i32 NstC Nst_check_1250_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1250 */
NstEXP u32 NstC Nst_1250_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1250 */
NstEXP i32 NstC Nst_1250_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1251 */
NstEXP i32 NstC Nst_check_1251_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1251 */
NstEXP u32 NstC Nst_1251_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1251 */
NstEXP i32 NstC Nst_1251_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1252 */
NstEXP i32 NstC Nst_check_1252_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1252 */
NstEXP u32 NstC Nst_1252_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1252 */
NstEXP i32 NstC Nst_1252_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1253 */
NstEXP i32 NstC Nst_check_1253_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1253 */
NstEXP u32 NstC Nst_1253_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1253 */
NstEXP i32 NstC Nst_1253_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1254 */
NstEXP i32 NstC Nst_check_1254_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1254 */
NstEXP u32 NstC Nst_1254_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1254 */
NstEXP i32 NstC Nst_1254_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1255 */
NstEXP i32 NstC Nst_check_1255_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1255 */
NstEXP u32 NstC Nst_1255_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1255 */
NstEXP i32 NstC Nst_1255_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1256 */
NstEXP i32 NstC Nst_check_1256_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1256 */
NstEXP u32 NstC Nst_1256_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1256 */
NstEXP i32 NstC Nst_1256_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1257 */
NstEXP i32 NstC Nst_check_1257_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1257 */
NstEXP u32 NstC Nst_1257_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1257 */
NstEXP i32 NstC Nst_1257_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for CP1258 */
NstEXP i32 NstC Nst_check_1258_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for CP1258 */
NstEXP u32 NstC Nst_1258_to_utf32(u8 *str);
/* Nst_FromUTF32Func for CP1258 */
NstEXP i32 NstC Nst_1258_from_utf32(u32 ch, u8 *str);

/* Nst_CheckBytesFunc for ISO-8859-1 */
NstEXP i32 NstC Nst_check_iso8859_1_bytes(u8 *str, usize len);
/* Nst_ToUTF32Func for ISO-8859-1 */
NstEXP u32 NstC Nst_iso8859_1_to_utf32(u8 *str);
/* Nst_FromUTF32Func for ISO-8859-1 */
NstEXP i32 NstC Nst_iso8859_1_from_utf32(u32 ch, u8 *str);

/** Translate the first character of a Unicode (UTF-16) string to UTF-8.
 *
 * @brief All arguments are expected to be valid.
 *
 * @param out_str: the buffer where the UTF-8 character is written
 * @param in_str: the input string to read
 * @param in_str_len: the length of the input string
 *
 * @return The function returns the number of bytes written or -1 on error,
 * no error is set.
 */
NstEXP i32 NstC Nst_utf16_to_utf8(i8 *out_str, u16 *in_str, usize in_str_len);

/** Translate a string to another encoding.
 *
 * @brief All pointers are expeced to be valid and not NULL except for `to_len`
 * that can be NULL if there is no need to get the length of the output string.
 *
 * @param from: the encoding of the given string
 * @param to: the encoding to translate the string to
 * @param from_buf: the initial string
 * @param from_len: the length in units of the given string (a unit is 1 byte
 * for char8_t strings, two bytes for char16_t strings etc.)
 * @param to_buf: the pointer where the newly translated string is put
 * @param to_len: the pointer where the length of the translated string is put,
 * it can be NULL
 *
 * @return true on success and false on failure. On failure the error is always
 * set.
 */
NstEXP bool NstC Nst_translate_cp(Nst_CP *from, Nst_CP *to, void *from_buf,
                                  usize from_len, void **to_buf, usize *to_len);

/** Checks the validity of the encoding of a string.
 *
 * @brief Both cp and str are expeced to be valid and non-NULL pointers.
 *
 * @param cp: the expeced encoding of the string
 * @param str: the string to check
 * @param str_len: the length in units of the string (a unit is 1 byte for
 * char8_t strings, two bytes for char16_t strings etc.)
 *
 * @return The index in units of the first invalid byte or -1 if the string is
 * correctly encoded. No error is set.
 */
NstEXP isize NstC Nst_check_string_cp(Nst_CP *cp, void *str, usize str_len);

/**
 * @return Returns the corresponding encoding structure given its ID. If an
 * invalid ID is given, NULL is returned and no error is set.
 */
NstEXP Nst_CP *NstC Nst_cp(Nst_CPID cpid);

#ifdef Nst_WIN
/**
 * @brief WINDOWS ONLY Returns the Nest code page ID of the local ANSI code
 * page. If the ANSI code page is not supported, Nst_CP_LATIN1 is returned.
 */
NstEXP Nst_CPID NstC Nst_acp();
#endif // !Nst_WIN

/** Translates a UTF-8 string to Unicode (UTF-16)
 *
 * @brief The new string is heap-allocated. str is assumed to be a valid
 * non-NULL pointer.
 *
 * @param str: the string to translate
 * @param len: the length of the string, if 0, it is calculated with strlen
 *
 * @return The function returns the new string or NULL on failure. If the
 * function fails, the error is set.
 */
NstEXP wchar_t *NstC Nst_char_to_wchar_t(i8 *str, usize len);

/** Translates a Unicode (UTF-16) string to UTF-8
*
* @brief The new string is heap-allocated. str is assumed to be a valid
* non-NULL pointer.
*
* @param str: the string to translate
* @param len: the length of the string, if 0, it is calculated with wcslen
*
* @return The function returns the new string or NULL on failure. If the
* function fails, the error is set.
*/
NstEXP i8 *NstC Nst_wchar_t_to_char(wchar_t *str, usize len);

/* Returns whether a code point is a noncharacter. */
NstEXP bool NstC Nst_is_valid_cp(u32 cp);

/**
 * @return The Nst_CPID deduced from the Byte Order Mark or Nst_CP_UNKNOWN if
 * no BOM was detected.
 */
NstEXP Nst_CPID NstC Nst_check_bom(i8 *str, usize len, i32 *bom_size);
/** Detects the encoding of a file.
 *
 * @brief If no valid encoding is detected, Nst_CP_LATIN1 is returned. No error
 * is set.
 */
NstEXP Nst_CPID NstC Nst_detect_encoding(i8 *str, usize len, i32 *bom_size);
/**
 * @return The encoding ID from a C string, if no matching encoding is found,
 * Nst_CP_UNKNOWN is returned. No error is set.
 */
NstEXP Nst_CPID NstC Nst_encoding_from_name(i8 *name);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ENCODING_H
