/**
 * @file file.h
 *
 * @brief `IOFile` object interface
 *
 * @author TheSilvered
 */

#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include "obj.h"
#include "encoding.h"

#ifdef Nst_MSVC
#include <windows.h>
#endif // !Nst_MSVC

/* Checks if `f` is closed. */
#define Nst_IOF_IS_CLOSED(f) Nst_HAS_FLAG(f, Nst_FLAG_IOFILE_IS_CLOSED)
/* Checks if `f` was opened in binary mode. */
#define Nst_IOF_IS_BIN(f) Nst_HAS_FLAG(f, Nst_FLAG_IOFILE_IS_BIN)
/* Checks if `f` is a TTY. */
#define Nst_IOF_IS_TTY(f) Nst_HAS_FLAG(f, Nst_FLAG_IOFILE_IS_TTY)
/* Checks if `f` can be written. */
#define Nst_IOF_CAN_WRITE(f) Nst_HAS_FLAG(f, Nst_FLAG_IOFILE_CAN_WRITE)
/* Checks if `f` can be read. */
#define Nst_IOF_CAN_READ(f) Nst_HAS_FLAG(f, Nst_FLAG_IOFILE_CAN_READ)
/* Checks if `f` can be seeked. */
#define Nst_IOF_CAN_SEEK(f) Nst_HAS_FLAG(f, Nst_FLAG_IOFILE_CAN_SEEK)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* [docs:link Nst_IO_BUF_FULL Nst_IOResult] */
/* [docs:link Nst_IO_EOF_REACHED Nst_IOResult] */
/* [docs:link Nst_IO_SUCCESS Nst_IOResult] */
/* [docs:link Nst_IO_ALLOC_FAILED Nst_IOResult] */
/* [docs:link Nst_IO_INVALID_ENCODING Nst_IOResult] */
/* [docs:link Nst_IO_INVALID_DECODING Nst_IOResult] */
/* [docs:link Nst_IO_OP_FAILED Nst_IOResult] */
/* [docs:link Nst_IO_CLOSED Nst_IOResult] */
/* [docs:link Nst_IO_ERROR Nst_IOResult] */

/* Enumeration of the possible IO return values. */
NstEXP typedef enum _Nst_IOResult {
    Nst_IO_BUF_FULL = 2,
    Nst_IO_EOF_REACHED = 1,
    Nst_IO_SUCCESS = 0,
    Nst_IO_ALLOC_FAILED = -1,
    Nst_IO_INVALID_ENCODING = -2,
    Nst_IO_INVALID_DECODING = -3,
    Nst_IO_OP_FAILED = -4,
    Nst_IO_CLOSED = -5,
    Nst_IO_ERROR = -6
} Nst_IOResult;

/* [docs:link Nst_SEEK_SET Nst_SeekWhence] */
/* [docs:link Nst_SEEK_CUR Nst_SeekWhence] */
/* [docs:link Nst_SEEK_END Nst_SeekWhence] */

/* Enumeration of the possible origins for seek file functions. */
NstEXP typedef enum _Nst_SeekWhence {
    Nst_SEEK_SET = 0,
    Nst_SEEK_CUR = 1,
    Nst_SEEK_END = 2
} Nst_SeekWhence;

/**
 * The type that represents a read function of a Nest file object.
 *
 * @brief This function shall read from the given file object `count`
 * characters (or `count` bytes when in binary mode) starting from the file
 * position indicator.
 *
 * @param buf: the buffer where the read text is written. If `buf_size` is
 * `0` this parameter should be interpreted as `i8 **` and a malloc'd buffer
 * of the right size shall be put in it. When the file is opened in normal
 * mode the contents of the buffer must be in `extUTF8` encoding and must
 * terminate with a NUL character.
 * @param buf_size: the size of `buf` in bytes, if set to `0` the buffer will
 * be allocated instead
 * @param count: the number of characters to read when opened in normal mode or
 * the number of bytes to read when opened in binary mode, a valid value can
 * be expected only when the function returns `Nst_IO_SUCCESS` or
 * `Nst_IO_EOF_REACHED`
 * @param buf_len: this is an out parameter set to the length in bytes of the
 * data written in `buf` ignoring the NUL character, it may be `NULL` to not
 * recieve the information
 * read when the file is opened in normal mode and to the number of bytes read
 * when opened in binary mode
 * @param f: the file to read
 *
 * @return This function shall return one of the following `Nst_IOResult`
 * variants:
 *! `Nst_IO_BUF_FULL` when `buf` is not allocated and cannot store all
 * requested characters or bytes.
 *! `Nst_IO_EOF_REACHED` when the end of the file has been reached.
 *! `Nst_IO_SUCCESS` when everything works correctly.
 *! `Nst_IO_ALLOC_FAILED` when the buffer fails to be allocated.
 *! `Nst_IO_INVALID_DECODING` when the text read cannot be decoded. This
 * variant cannot be returned if the file is in binary mode. When it is
 * returned `Nst_io_result_set_details` must be called to communicate the
 * appropriate information.
 *! `Nst_IO_OP_FAILED` if the file does not support reading.
 *! `Nst_IO_CLOSED` if the file is closed.
 *! `Nst_IO_ERROR` for any other error that might occur.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_read_f)(i8 *buf, usize buf_size,
                                                 usize count, usize *buf_len,
                                                 Nst_Obj *f);
/**
 * The type that represents a write function of a Nest file object.
 *
 * @brief This function shall write the contents of buf to a file starting from
 * the file position indicator and overwriting any previous content. If count is
 * not `NULL` it is filled with the number of characters written (or the number
 * of bytes if the file is in binary mode). `buf` shall contain UTF-8 text that
 * allows invalid characters under U+10FFFF.
 *
 * @param buf: the content to write to the file
 * @param buf_len: the length in bytes of `buf`
 * @param count: an out parameter set to the number of characters written when
 * the file is opened in normal mode or to the number of bytes written when
 * it is in binary mode, it may be `NULL` to not recieve the information,a
 * valid value can be expected only when the function returns `Nst_IO_SUCCESS`
 * @param f: the file to write to
 *
 * @return This function shall return one of the following `Nst_IOResult`
 * variants:
 *! `Nst_IO_SUCCESS` when the function successfully writes the characters to
 * the file.
 *! `Nst_IO_ALLOC_FAILED` if a memory allocation fails.
 *! `Nst_IO_INVALID_ENCODING` if a character cannot be encoded in the encoding
 * the file is opened in. This variant can only be returned when the file is
 * not binary. When it is returned `Nst_io_result_set_details` must be called
 * to communicate the appropriate information.
 *! `Nst_IO_OP_FAILED` if the file does not support writing.
 *! `Nst_IO_CLOSED` if the file is closed.
 *! `Nst_IO_ERROR` for any other error that might occur.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_write_f)(i8 *buf, usize buf_len,
                                                  usize *count, Nst_Obj *f);
/**
 * The type that represents a flush function of a Nest file object.
 *
 * @brief This function shall write any buffered bytes to the file.
 *
 * @param f: the file to flush
 *
 * @return This function shall return one of the following `Nst_IOResult`
 * variants:
 *! `Nst_IO_CLOSED` when the file is closed.
 *! `Nst_IO_OP_FAILED` if the file does not support writing.
 *! `Nst_IO_SUCCESS` if the function exits successfully.
 *! `Nst_IO_ALLOC_FAILED` if a memory allocation fails.
 *! `Nst_IO_ERROR` for any other error.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_flush_f)(Nst_Obj *f);
/**
 * The type that represents a tell function of a Nest file object.
 *
 * @brief This function shall get the current position in bytes from
 * the start of the file of the file-position indicator.
 *
 * @param f: the file to get the position from
 * @param pos: the pointer filled with the retrived position, a valid value can
 * be expected only when the function returns `Nst_IO_SUCCESS`
 *
 * @return This function shall return one of the following `Nst_IOResult`
 * variants:
 *! `Nst_IO_CLOSED` when the file is closed.
 *! `Nst_IO_OP_FAILED` if the file does not support seeking.
 *! `Nst_IO_SUCCESS` if the function exits successfully.
 *! `Nst_IO_ERROR` for any other error.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_tell_f)(Nst_Obj *f,
                                                 usize *pos);
/**
 * The type that represents a seek function of a Nest file object.
 *
 * @brief This function shall move the file-position indicator. `Nst_SEEK_SET` is the start of the file,
 * `Nst_SEEK_CUR` is the current position of the file-position indicator and
 * `Nst_SEEK_END` is the end of the file.
 *
 * @param origin: where to calculate the offset from, `Nst_SEEK_SET` is the
 * start of the file, `Nst_SEEK_CUR` is the current position of the indicator
 * and `Nst_SEEK_END` is the end of the file
 * @param offset: an offset in bytes from `origin` to move the indicator
 * @param f: the file to move the indicator of
 *
 * @return This function shall return one of the following `Nst_IOResult`
 * variants:
 *! `Nst_IO_CLOSED` when the file is closed.
 *! `Nst_IO_OP_FAILED` if the file does not support seeking.
 *! `Nst_IO_SUCCESS` if the function exits successfully.
 *! `Nst_IO_ERROR` for any other error.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_seek_f)(Nst_SeekWhence origin,
                                                 isize offset, Nst_Obj *f);
/**
 * The type that represents a close function of a Nest file object.
 *
 * @brief This function shall close the given file and free any allocated
 * memory.
 *
 * @param f: the file to close
 *
 * @return This function shall return one of the following `Nst_IOResult`
 * variants:
 *! `Nst_IO_CLOSED` when the file was already closed.
 *! `Nst_IO_SUCCESS` if the function exits successfully.
 *! `Nst_IO_ERROR` for any other error.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_close_f)(Nst_Obj *f);

/**
 * @brief A structure representing the functions necessary to operate a Nest
 * file object.
 */
NstEXP typedef struct _Nst_IOFuncSet {
    Nst_IOFile_read_f read;
    Nst_IOFile_write_f write;
    Nst_IOFile_flush_f flush;
    Nst_IOFile_tell_f tell;
    Nst_IOFile_seek_f seek;
    Nst_IOFile_close_f close;
} Nst_IOFuncSet;

#ifdef Nst_MSVC

#define _Nst_WIN_STDIN_BUF_SIZE 2048

/**
 * @brief WINDOWS ONLY A structure representing the standard input file on
 * Windows.
 */
NstEXP typedef struct _Nst_StdIn {
    HANDLE hd;
    wchar_t buf[_Nst_WIN_STDIN_BUF_SIZE];
    FILE *fp;
    i32 buf_size;
    i32 buf_ptr;
} Nst_StdIn;

extern Nst_StdIn Nst_stdin;

#endif // !Nst_MSVC

/* The flags of a IO file. */
NstEXP typedef enum _Nst_IOFileFlag {
    Nst_FLAG_IOFILE_IS_CLOSED = Nst_FLAG(1),
    Nst_FLAG_IOFILE_IS_BIN    = Nst_FLAG(2),
    Nst_FLAG_IOFILE_CAN_WRITE = Nst_FLAG(3),
    Nst_FLAG_IOFILE_CAN_READ  = Nst_FLAG(4),
    Nst_FLAG_IOFILE_CAN_SEEK  = Nst_FLAG(5),
    Nst_FLAG_IOFILE_IS_TTY    = Nst_FLAG(6)
} Nst_IOFileFlag;

/**
 * Creates a new `IOFile` object from a C file pointer.
 *
 * @param value: the value of the new object
 * @param bin: if the file is in binary mode
 * @param read: whether the file can be read
 * @param write: whether the file can be written
 * @param encoding: the encoding of the opened file, ignored when bin is true
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_iof_new(FILE *value, bool bin, bool read,
                                 bool write, Nst_Encoding *encoding);
/**
 * Creates a new `IOFile` object that is not a C file pointer.
 *
 * @param value: the value of the new object
 * @param bin: if the file is in binary mode
 * @param read: whether the file can be read
 * @param write: whether the file can be written
 * @param seek: whether the file can be sought
 * @param encoding: the encoding of the opened file, ignored when `bin` is
 * `true`
 * @param func_set: custom functions used to read the file
 *
 * @return The new object on success or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_iof_new_fake(void *value, bool bin, bool read,
                                      bool write, bool seek,
                                      Nst_Encoding *encoding,
                                      Nst_IOFuncSet func_set);

/* Get the `Nst_IOFuncSet` of a file. */
NstEXP Nst_IOFuncSet *NstC Nst_iof_func_set(Nst_Obj *f);
/* Get the file descriptor, if it's negative the file is fake. */
NstEXP int NstC Nst_iof_fd(Nst_Obj *f);
/**
 * @brief Get a pointer to the file's data. If the descriptor is positive this
 * is of type `FILE *`.
 */
NstEXP void *NstC Nst_iof_fp(Nst_Obj *f);
/* Get the encoding of a file. */
NstEXP Nst_Encoding *NstC Nst_iof_encoding(Nst_Obj *f);

void NstC _Nst_iofile_destroy(Nst_Obj *obj);

/* Calls the read function of the file, see `Nst_IOFile_read_f`. */
NstEXP Nst_IOResult NstC Nst_fread(i8 *buf, usize buf_size, usize count,
                                   usize *buf_len, Nst_Obj *f);
/* Calls the write function of the file, see `Nst_IOFile_write_f`. */
NstEXP Nst_IOResult NstC Nst_fwrite(i8 *buf, usize buf_len, usize *count,
                                    Nst_Obj *f);
/* Calls the flush function of the file, see `Nst_IOFile_flush_f`. */
NstEXP Nst_IOResult NstC Nst_fflush(Nst_Obj *f);
/* Calls the tell function of the file, see `Nst_IOFile_tell_f`. */
NstEXP Nst_IOResult NstC Nst_ftell(Nst_Obj *f, usize *pos);
/* Calls the seek function of the file, see `Nst_IOFile_seek_f`. */
NstEXP Nst_IOResult NstC Nst_fseek(Nst_SeekWhence origin, isize offset,
                                   Nst_Obj *f);
/* Calls the close function of the file, see `Nst_IOFile_close_f`. */
NstEXP Nst_IOResult NstC Nst_fclose(Nst_Obj *f);

/* Read function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_read(i8 *buf, usize buf_size, usize count,
                                       usize *buf_len, Nst_Obj *f);
/* Write function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_write(i8 *buf, usize buf_len, usize *count,
                                        Nst_Obj *f);
/* Flush function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_flush(Nst_Obj *f);
/* Tell function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_tell(Nst_Obj *f, usize *pos);

NstEXP Nst_IOResult NstC Nst_FILE_seek(Nst_SeekWhence origin, isize offset,
                                       Nst_Obj *f);
/* Close function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_close(Nst_Obj *f);

/**
 * Gets the details of the `Nst_IOResult` returned by the functions.
 *
 * @brief This function can only be called when the returned `Nst_IOResult` is
 * either `Nst_IO_INVALID_ENCODING` or `Nst_IO_INVALID_DECODING`. If the result
 * is the former `ill_encoded_ch` will be the code point that could not be
 * encoded, otherwise, if the result is the latter, `ill_encoded_ch` will
 * represent the byte that could not be decoded. Similarly `encoding_name` is
 * the encoding that failed to encode the code point for
 * `Nst_IO_INVALID_ENCODING` and the name of the one that failed to decode the
 * byte for `Nst_IO_INVALID_DECODING`.
 *
 * @param ill_encoded_ch: variable filled with the character that failed to
 * encode or decode, may be `NULL`
 * @param position: variable filled with the position in the file of the
 * encoding error, may be `NULL`
 * @param encoding_name: variable filled with the name of the encoding, may be
 * `NULL`
 */
NstEXP void NstC Nst_io_result_get_details(u32 *ill_encoded_ch,
                                           usize *position,
                                           const i8 **encoding_name);
/* Sets the values returned with `Nst_io_result_get_details`. */
NstEXP void NstC Nst_io_result_set_details(u32 ill_encoded_ch,
                                           usize position,
                                           const i8 *encoding_name);
/**
 * Opens a file given a path that can contain unicode characters in UTF-8.
 *
 * @param path: the path to the file
 * @param mode: the mode to open the file with
 *
 * @return The file pointer on success and `NULL` on failure. The error is set
 * only if a `Memory Error` occurs.
 */
NstEXP FILE *NstC Nst_fopen_unicode(i8 *path, const i8 *mode);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FILE_H
