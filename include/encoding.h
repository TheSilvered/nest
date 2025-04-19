/**
 * @file encoding.h
 *
 * @brief Functions to manage various encodings
 *
 * @author TheSilvered
 */

#ifndef ENCODING_H
#define ENCODING_H

#include "typedefs.h"

/* Maximum size of a multi-byte character across all supported encodings. */
#define Nst_ENCODING_MULTIBYTE_MAX_SIZE 4
/* Maximum size of the BOM across all supported encodings. */
#define Nst_ENCODING_BOM_MAX_SIZE 4

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* [docs:link Nst_EID_UNKNOWN Nst_EncodingID] */
/* [docs:link Nst_EID_LATIN1 Nst_EncodingID] */
/* [docs:link Nst_EID_ISO8859_1 Nst_EncodingID] */
/* [docs:link Nst_EID_EXT_UTF8 Nst_EncodingID] */
/* [docs:link Nst_EID_EXT_UTF16 Nst_EncodingID] */

/**
 * The supported encodings in Nest.
 *
 * @brief `Nst_EID_UNKNOWN` is `-1`; `Nst_EID_LATIN1` and `Nst_EID_ISO8859_1` are
 * equivalent.
 *
 * @brief Note: `Nst_EID_EXT_UTF8` is a UTF-8 encoding that allows surrogates to
 * be encoded.
 *
 * @brief Note: `Nst_EID_EXT_UTF16` along with the little and big endian
 * versions are UTF-16 encodings that allow for unpaired surrogates with the
 * only constraint being that a high surrogate cannot be the last character.
 */
NstEXP typedef enum _Nst_EncodingID {
    Nst_EID_UNKNOWN = -1,
    Nst_EID_ASCII,
    Nst_EID_UTF8,
    Nst_EID_EXT_UTF8,
    Nst_EID_UTF16,
    Nst_EID_UTF16BE,
    Nst_EID_UTF16LE,
    Nst_EID_EXT_UTF16,
    Nst_EID_EXT_UTF16BE,
    Nst_EID_EXT_UTF16LE,
    Nst_EID_UTF32,
    Nst_EID_UTF32BE,
    Nst_EID_UTF32LE,
    Nst_EID_1250,
    Nst_EID_1251,
    Nst_EID_1252,
    Nst_EID_1253,
    Nst_EID_1254,
    Nst_EID_1255,
    Nst_EID_1256,
    Nst_EID_1257,
    Nst_EID_1258,
    Nst_EID_LATIN1,
    Nst_EID_ISO8859_1 = Nst_EID_LATIN1
} Nst_EncodingID;

/**
 * The signature of a function that checks the length of the first character in
 * a string of a certain encoding.
 *
 * @brief Note: if the length is unknown but it is certain that the string
 * contains at least one character you can use
 * `Nst_ENCODING_MULTIBYTE_MAX_SIZE` to ensure that the function does not fail
 * due to a length too small.
 *
 * @return The length in bytes of the first character of the string. If the
 * sequence of bytes is not valid or incomplete this function returns `-1`.
 */
NstEXP typedef i32 (*Nst_CheckBytesFunc)(void *str, usize len);

/**
 * @brief The signature of a function that returns the code point of the first
 * character in a string decoded with a certain encoding.
 *
 * @brief Warning: `str` is expected to be a valid string, you can check that
 * it is valid with a function of type `Nst_CheckBytesFunc`. Since the string
 * is assumed to be valid this function never fails.
 */
NstEXP typedef u32 (*Nst_ToUTF32Func)(void *str);

/**
 * @brief The signature of a function that encodesa a code point with a certain
 * encoding writing the output to a buffer.
 *
 * @brief Warning: `buf` is expected to be large enough to hold the full
 * character, if the final length of the character is unknown you can ensure
 * that `buf` has space for at least `Nst_ENCODING_MULTIBYTE_MAX_SIZE` bytes.
 * This type of functions are guaranteed to never write more than
 * `Nst_ENCODING_MULTIBYTE_MAX_SIZE` bytes.
 *
 * @return The number of bytes written. If the character could not be encoded
 * this function returns `-1`.
 */
NstEXP typedef i32 (*Nst_FromUTF32Func)(u32 ch, void *buf);

/**
 * The structure that represents an encoding.
 *
 * @param ch_size: the size of one unit in bytes (e.g. is 1 in UTF-8 but 2 in
 * UTF-16)
 * @param mult_max_sz: the size in bytes of the longest character
 * @param mult_min_sz: the size in bytes of the shortest character (usually the
 * same as `ch_size`)
 * @param name: the name of the encoding displayed in errors
 * @param bom: the Byte Order Mark of the encoding, is set to `NULL` if it does
 * not have one
 * @param bom_size: the length of `bom`, set to `0` if it is `NULL`
 * @param check_bytes: the `Nst_CheckBytesFunc` function of the encoding
 * @param to_utf32: the `Nst_ToUTF32Func` function of the encoding
 * @param from_utf32: the `Nst_FromUTF32Func` function of the encoding
 */
NstEXP typedef struct _Nst_Encoding {
    const usize ch_size;
    const usize mult_max_sz;
    const usize mult_min_sz;
    const char *name;
    const char *bom;
    const usize bom_size;
    const Nst_CheckBytesFunc check_bytes;
    const Nst_ToUTF32Func to_utf32;
    const Nst_FromUTF32Func from_utf32;
} Nst_Encoding;

extern Nst_Encoding Nst_encoding_ascii;
extern Nst_Encoding Nst_encoding_utf8;
extern Nst_Encoding Nst_encoding_ext_utf8;
extern Nst_Encoding Nst_encoding_utf16;
extern Nst_Encoding Nst_encoding_utf16be;
extern Nst_Encoding Nst_encoding_utf16le;
extern Nst_Encoding Nst_encoding_utf32;
extern Nst_Encoding Nst_encoding_utf32be;
extern Nst_Encoding Nst_encoding_utf32le;
extern Nst_Encoding Nst_encoding_1250;
extern Nst_Encoding Nst_encoding_1251;
extern Nst_Encoding Nst_encoding_1252;
extern Nst_Encoding Nst_encoding_1253;
extern Nst_Encoding Nst_encoding_1254;
extern Nst_Encoding Nst_encoding_1255;
extern Nst_Encoding Nst_encoding_1256;
extern Nst_Encoding Nst_encoding_1257;
extern Nst_Encoding Nst_encoding_1258;
extern Nst_Encoding Nst_encoding_iso8859_1;

/* `Nst_CheckBytesFunc` for ASCII. */
NstEXP i32 NstC Nst_check_ascii_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for ASCII */
NstEXP u32 NstC Nst_ascii_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for ASCII */
NstEXP i32 NstC Nst_ascii_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for UTF-8. */
NstEXP i32 NstC Nst_check_utf8_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for UTF-8. */
NstEXP u32 NstC Nst_utf8_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for UTF-8. */
NstEXP i32 NstC Nst_utf8_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for extUTF-8. */
NstEXP i32 NstC Nst_check_ext_utf8_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for extUTF-8. */
NstEXP u32 NstC Nst_ext_utf8_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for extUTF-8. */
NstEXP i32 NstC Nst_ext_utf8_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for UTF-16. */
NstEXP i32 NstC Nst_check_utf16_bytes(u16 *str, usize len);
/* `Nst_ToUTF32Func` for UTF-16. */
NstEXP u32 NstC Nst_utf16_to_utf32(u16 *str);
/* `Nst_FromUTF32Func` for UTF-16. */
NstEXP i32 NstC Nst_utf16_from_utf32(u32 ch, u16 *str);

/* `Nst_CheckBytesFunc` for UTF-16BE. */
NstEXP i32 NstC Nst_check_utf16be_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for UTF-16BE. */
NstEXP u32 NstC Nst_utf16be_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for UTF-16BE. */
NstEXP i32 NstC Nst_utf16be_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for UTF-16LE. */
NstEXP i32 NstC Nst_check_utf16le_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for UTF-16LE. */
NstEXP u32 NstC Nst_utf16le_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for UTF-16LE. */
NstEXP i32 NstC Nst_utf16le_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for extUTF-16. */
NstEXP i32 NstC Nst_check_ext_utf16_bytes(u16 *str, usize len);
/* `Nst_ToUTF32Func` for extUTF-16. */
NstEXP u32 NstC Nst_ext_utf16_to_utf32(u16 *str);
/* `Nst_FromUTF32Func` for extUTF-16. */
NstEXP i32 NstC Nst_ext_utf16_from_utf32(u32 ch, u16 *str);

/* `Nst_CheckBytesFunc` for extUTF-16BE. */
NstEXP i32 NstC Nst_check_ext_utf16be_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for extUTF-16BE. */
NstEXP u32 NstC Nst_ext_utf16be_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for extUTF-16BE. */
NstEXP i32 NstC Nst_ext_utf16be_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for extUTF-16LE. */
NstEXP i32 NstC Nst_check_ext_utf16le_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for extUTF-16LE. */
NstEXP u32 NstC Nst_ext_utf16le_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for extUTF-16LE. */
NstEXP i32 NstC Nst_ext_utf16le_from_utf32(u32 ch, u8 *str);


/* `Nst_CheckBytesFunc` for UTF-32. */
NstEXP i32 NstC Nst_check_utf32_bytes(u32 *str, usize len);
/* `Nst_ToUTF32Func` for UTF-32. */
NstEXP u32 NstC Nst_utf32_to_utf32(u32 *str);
/* `Nst_FromUTF32Func` for UTF-32. */
NstEXP i32 NstC Nst_utf32_from_utf32(u32 ch, u32 *str);

/* `Nst_CheckBytesFunc` for UTF-32BE. */
NstEXP i32 NstC Nst_check_utf32be_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for UTF-32BE. */
NstEXP u32 NstC Nst_utf32be_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for UTF-32BE. */
NstEXP i32 NstC Nst_utf32be_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for UTF-32LE. */
NstEXP i32 NstC Nst_check_utf32le_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for UTF-32LE. */
NstEXP u32 NstC Nst_utf32le_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for UTF-32LE. */
NstEXP i32 NstC Nst_utf32le_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1250. */
NstEXP i32 NstC Nst_check_1250_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1250. */
NstEXP u32 NstC Nst_1250_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1250. */
NstEXP i32 NstC Nst_1250_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1251. */
NstEXP i32 NstC Nst_check_1251_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1251. */
NstEXP u32 NstC Nst_1251_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1251. */
NstEXP i32 NstC Nst_1251_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1252. */
NstEXP i32 NstC Nst_check_1252_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1252. */
NstEXP u32 NstC Nst_1252_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1252. */
NstEXP i32 NstC Nst_1252_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1253. */
NstEXP i32 NstC Nst_check_1253_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1253. */
NstEXP u32 NstC Nst_1253_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1253. */
NstEXP i32 NstC Nst_1253_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1254. */
NstEXP i32 NstC Nst_check_1254_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1254. */
NstEXP u32 NstC Nst_1254_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1254. */
NstEXP i32 NstC Nst_1254_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1255. */
NstEXP i32 NstC Nst_check_1255_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1255. */
NstEXP u32 NstC Nst_1255_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1255. */
NstEXP i32 NstC Nst_1255_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1256. */
NstEXP i32 NstC Nst_check_1256_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1256. */
NstEXP u32 NstC Nst_1256_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1256. */
NstEXP i32 NstC Nst_1256_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1257. */
NstEXP i32 NstC Nst_check_1257_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1257. */
NstEXP u32 NstC Nst_1257_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1257. */
NstEXP i32 NstC Nst_1257_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for CP1258. */
NstEXP i32 NstC Nst_check_1258_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for CP1258. */
NstEXP u32 NstC Nst_1258_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for CP1258. */
NstEXP i32 NstC Nst_1258_from_utf32(u32 ch, u8 *str);

/* `Nst_CheckBytesFunc` for ISO-8859-1. */
NstEXP i32 NstC Nst_check_iso8859_1_bytes(u8 *str, usize len);
/* `Nst_ToUTF32Func` for ISO-8859-1. */
NstEXP u32 NstC Nst_iso8859_1_to_utf32(u8 *str);
/* `Nst_FromUTF32Func` for ISO-8859-1. */
NstEXP i32 NstC Nst_iso8859_1_from_utf32(u32 ch, u8 *str);

/**
 * Translate the first character of a Unicode (UTF-16) string to UTF-8.
 *
 * @param out_str: the buffer where the UTF-8 character is written
 * @param in_str: the input string to read
 * @param in_str_len: the length of the input string
 *
 * @return The function returns the number of bytes written or `-1` on error,
 * no error is set.
 */
NstEXP i32 NstC Nst_utf16_to_utf8(u8 *out_str, u16 *in_str, usize in_str_len);

/**
 * Translate a string to another encoding.
 *
 * @brief All pointers are expected to be valid and not `NULL` except for
 * `to_len` that can be `NULL` if there is no need to get the length of the
 * output string.
 *
 * @param from: the encoding of the given string
 * @param to: the encoding to translate the string to
 * @param from_buf: the initial string
 * @param from_len: the length in units of the given string (a unit is 1 byte
 * for `char8_t` strings, two bytes for `char16_t` strings etc.)
 * @param to_buf: the pointer where the newly translated string is put
 * @param to_len: the pointer where the length of the translated string is put,
 * it can be `NULL`
 *
 * @return `true` on success and `false` on failure. On failure the error is
 * set.
 */
NstEXP bool NstC Nst_encoding_translate(Nst_Encoding *from, Nst_Encoding *to,
                                        void *from_buf, usize from_len,
                                        void **to_buf, usize *to_len);

/**
 * Checks the validity of the encoding of a string.
 *
 * @param encoding: the expected encoding of the string
 * @param str: the string to check
 * @param str_len: the length in units of the string (a unit is 1 byte for
 * `char8_t` strings, two bytes for `char16_t` strings etc.)
 *
 * @return The index in units of the first invalid byte or `-1` if the string
 * is correctly encoded. No error is set.
 */
NstEXP isize NstC Nst_encoding_check(Nst_Encoding *encoding, void *str,
                                     usize str_len);

/**
 * Gets the length in characters of an encoded string.
 *
 * @param encoding: the encoding of the string
 * @param str: the string to get the length of
 * @param str_len: the length in units of the string (a unit is 1 byte for
 * `char8_t` strings, two bytes for `char16_t` strings etc.)
 *
 * @return The length in characters of the string or -1 on failure. The error
 * is set.
 */
NstEXP isize NstC Nst_encoding_char_len(Nst_Encoding *encoding, void *str,
                                        usize str_len);

/**
 * Gets the length in characters of a UTF-8-encoded string.
 *
 * @brief Note: this function assumes that the string is valid UTF-8 and does
 * no error checking. Use `Nst_encoding_check` to check it or
 * `Nst_encoding_char_len` to get the length in characters safely.
 *
 * @param str: the string to get the length of
 * @param str_len: the length in bytes of the string
 *
 * @return The length in characters of the string. No error is set.
 */
NstEXP usize NstC Nst_encoding_utf8_char_len(u8 *str, usize str_len);

/**
 * @return The corresponding encoding structure given its ID. If an invalid ID
 * is given, `NULL` is returned and no error is set.
 */
NstEXP Nst_Encoding *NstC Nst_encoding(Nst_EncodingID eid);

#ifdef Nst_MSVC
/**
 * @brief WINDOWS ONLY Returns the Nest code page ID of the local ANSI code
 * page. If the ANSI code page is not supported, `Nst_EID_LATIN1` is returned.
 */
NstEXP Nst_EncodingID NstC Nst_acp(void);
#endif // !Nst_MSVC

/* [docs:link strlen <https://man7.org/linux/man-pages/man3/strlen.3.html>] */
/* [docs:link wcslen <https://man7.org/linux/man-pages/man3/wcslen.3.html>] */

/**
 * Translates a UTF-8 string to Unicode (UTF-16).
 *
 * @brief The new string is heap-allocated. `str` is assumed to be a valid
 * non-NULL pointer.
 *
 * @param str: the string to translate
 * @param len: the length of the string, if 0, it is calculated with `strlen`
 *
 * @return The function returns the new string or NULL on failure. If the
 * function fails, the error is set.
 */
NstEXP wchar_t *NstC Nst_char_to_wchar_t(const char *str, usize len);

/**
 * Translates a Unicode (UTF-16) string to UTF-8.
 *
 * @brief The new string is heap-allocated. `str` is assumed to be a valid
 * non-NULL pointer.
 *
 * @param str: the string to translate
 * @param len: the length of the string, if `0`, it is calculated with `wcslen`
 *
 * @return The function returns the new string or `NULL` on failure. If the
 * function fails, the error is set.
 */
NstEXP char *NstC Nst_wchar_t_to_char(wchar_t *str, usize len);

/**
 * @brief Returns whether a code point is valid. A valid code point is smaller
 * than or equal to U+10FFFF and is not a high or low surrogate.
 */
NstEXP bool NstC Nst_is_valid_cp(u32 cp);

/* Returns whether a code is a non character. */
NstEXP bool NstC Nst_is_non_character(u32 cp);

/**
 * @return The `Nst_EncodingID` deduced from the Byte Order Mark or
 * `Nst_EID_UNKNOWN` if no BOM was detected.
 */
NstEXP Nst_EncodingID NstC Nst_check_bom(char *str, usize len, i32 *bom_size);
/**
 * Detects the encoding of a file.
 *
 * @brief If no valid encoding is detected, `Nst_EID_LATIN1` is returned.
 * No error is set.
 */
NstEXP Nst_EncodingID NstC Nst_encoding_detect(char *str, usize len,
                                               i32 *bom_size);
/**
 * @return The encoding ID from a C string, if no matching encoding is found,
 * `Nst_EID_UNKNOWN` is returned. No error is set.
 */
NstEXP Nst_EncodingID NstC Nst_encoding_from_name(const char *name);
/**
 * @return An encoding ID where `ch_size` is one byte. If the given encoding ID
 * has a `ch_size` of one byte already the encoding ID itself is returned.
 * Otherwies the little endian version is always returned.
 */
NstEXP Nst_EncodingID NstC Nst_encoding_to_single_byte(Nst_EncodingID encoding);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !ENCODING_H
