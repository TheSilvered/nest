/**
 * @file file.h
 *
 * @brief Nest file object.
 *
 * @author TheSilvered
 */

#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include "obj.h"
#include "encoding.h"

#ifdef Nst_WIN
#include <windows.h>
#endif // !Nst_WIN

#define _Nst_RBUF_SIZE 512

/* Casts ptr to a Nst_IOFileObj *. */
#define IOFILE(ptr) ((Nst_IOFileObj *)(ptr))

/* Checks if f is closed. */
#define Nst_IOF_IS_CLOSED(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_IS_CLOSED)
/* Checks if f was opened in binary mode. */
#define Nst_IOF_IS_BIN(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_IS_BIN)
/* Checks if f is a TTY. */
#define Nst_IOF_IS_TTY(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_IS_TTY)
/* Checks if f can be written. */
#define Nst_IOF_CAN_WRITE(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_CAN_WRITE)
/* Checks if f can be read. */
#define Nst_IOF_CAN_READ(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_CAN_READ)
/* Checks if f can be seeked. */
#define Nst_IOF_CAN_SEEK(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_CAN_SEEK)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* Enumeration of the possible IO return values. */
typedef enum _Nst_IOResult {
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

/* Enumeration of the possible origins for seek file functions. */
typedef enum _Nst_SeekWhence {
    Nst_SEEK_SET = 0,
    Nst_SEEK_CUR = 1,
    Nst_SEEK_END = 2
} Nst_SeekWhence;

struct _Nst_IOFileObj;

/**
 * The type that represents a read function of a Nest file object.
 *
 * @brief This function shall read from the given file object count characters
 * or bytes when in binary mode. buf shall be interpreted as i8 ** instead of
 * i8 * and a new buffer shall be allocated with Nst_malloc or similar
 * functions. The buffer shall contain UTF8-encoded text. When buf_len is not
 * NULL the function shall fill it with the number of characters written (or
 * bytes if it is in binary mode).
 *
 * @return This function shall return any Nst_IOResult variant except for
 * Nst_IO_INVALID_ENCODING as follows:
 * Nst_IO_BUF_FULL when buf is not allocated and cannot store all requested
 * characters or bytes.
 * Nst_IO_EOF_REACHED when the end of the file has been reached.
 * Nst_IO_SUCCESS when everything works correctly.
 * Nst_IO_ALLOC_FAILED when the buffer fails to be allocated.
 * Nst_IO_INVALID_DECODING when the text read cannot be decoded. This variant
 * cannot be returned if the file is in binary mode. When it is returned
 * Nst_io_result_set_details must be called.
 * Nst_IO_OP_FAILED if the file does not support reading.
 * Nst_IO_CLOSED if the file is closed.
 * Nst_IO_ERROR for any other error that might occur.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_read_f)(i8 *buf, usize buf_size,
                                                 usize count, usize *buf_len,
                                                 struct _Nst_IOFileObj *f);
/**
 * The type that represents a write function of a Nest file object.
 *
 * @brief This function shall write the contents of buf to a file. If count is
 * not NULL it is filled with the number of characters written (or the number
 * of bytes if the file is in binary mode). buf shall contain UTF-8 text that
 * allows invalid characters under U+10FFFF.
 *
 * @return This function shall not return Nst_IO_BUF_FULL, Nst_IO_EOF_REACHED
 * and Nst_IO_INVALID_DECODING variants of Nst_IOResult. The other ones shall
 * be returned as follows:
 * Nst_IO_SUCCESS when the function succesfully writes the characters to the
 * file.
 * Nst_IO_ALLOC_FAILED if a memory allocation fails.
 * Nst_IO_INVALID_ENCODING if a character cannot be encoded in the encoding the
 * file is opended in. This variant can only be returned when the file is not
 * binary. When it is returned Nst_io_result_set_details must be called.
 * Nst_IO_OP_FAILED if the file does not support writing.
 * Nst_IO_CLOSED if the file is closed.
 * Nst_IO_ERROR for any other error that might occur.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_write_f)(i8 *buf, usize buf_len,
                                                  usize *count,
                                                  struct _Nst_IOFileObj *f);
/**
 * The type that represents a flush function of a Nest file object.
 *
 * @brief This function shall write any buffered bytes to the file.
 *
 * @return This function shall return only either Nst_IO_CLOSED, Nst_IO_ERROR,
 * Nst_IO_OP_FAILED, Nst_IO_SUCCESS or Nst_IO_ALLOC_FAILED as follows:
 * Nst_IO_CLOSED when the file is closed.
 * Nst_IO_OP_FAILED if the file does not support writing.
 * Nst_IO_SUCCESS if the function exits successfully.
 * Nst_IO_ALLOC_FAILED if a memory allocation fails.
 * Nst_IO_ERROR for any other error.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_flush_f)(struct _Nst_IOFileObj *f);
/**
 * The type that represents a tell function of a Nest file object.
 *
 * @brief This function shall fill pos with the current position in bytes from
 * the start of the file of the file-position indicator.
 *
 * @return This function shall return only either Nst_IO_CLOSED, Nst_IO_ERROR,
 * Nst_IO_OP_FAILED or Nst_IO_SUCCESS as follows:
 * Nst_IO_CLOSED when the file is closed.
 * Nst_IO_OP_FAILED if the file does not support seeking.
 * Nst_IO_SUCCESS if the function exits successfully.
 * Nst_IO_ERROR for any other error.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_tell_f)(struct _Nst_IOFileObj *f,
                                                 usize *pos);
/**
 * The type that represents a seek function of a Nest file object.
 *
 * @brief This function shall move the file-position indicator by an offset
 * starting from origin. Nst_SEEK_SET is the start of the file, Nst_SEEK_CUR
 * is the current position of the file-position indicator and Nst_SEEK_END is
 * the end of the file.
 *
 * @return This function shall return only either Nst_IO_CLOSED, Nst_IO_ERROR,
 * Nst_IO_OP_FAILED or Nst_IO_SUCCESS as follows:
 * Nst_IO_CLOSED when the file is closed.
 * Nst_IO_OP_FAILED if the file does not support seeking.
 * Nst_IO_SUCCESS if the function exits successfully.
 * Nst_IO_ERROR for any other error.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_seek_f)(Nst_SeekWhence origin,
                                                 isize offset,
                                                 struct _Nst_IOFileObj *f);
/**
 * The type that represents a seek function of a Nest file object.
 *
 * @brief This function shall move the file-position indicator by an offset
 * starting from origin. Nst_SEEK_SET is the start of the file, Nst_SEEK_CUR
 * is the current position of the file-position indicator and Nst_SEEK_END is
 * the end of the file.
 *
 * @return This function shall return only either Nst_IO_CLOSED, Nst_IO_ERROR
 * or Nst_IO_SUCCESS as follows:
 * Nst_IO_CLOSED when the file was already closed.
 * Nst_IO_SUCCESS if the function exits successfully.
 * Nst_IO_ERROR for any other error.
 */
NstEXP typedef Nst_IOResult (*Nst_IOFile_close_f)(struct _Nst_IOFileObj *f);

/**
 * @breif A structure representing the functions necessary to operate a Nest
 * file object.
 */
typedef struct _Nst_IOFuncSet {
    Nst_IOFile_read_f read;
    Nst_IOFile_write_f write;
    Nst_IOFile_flush_f flush;
    Nst_IOFile_tell_f tell;
    Nst_IOFile_seek_f seek;
    Nst_IOFile_close_f close;
} Nst_IOFuncSet;

#ifdef Nst_WIN

/**
 * @breif WINDOWS ONLY A structure representing the stdandard input file on
 * Windows.
 */
typedef struct _Nst_StdIn {
    HANDLE hd;
    wchar_t buf[1024];
    FILE *fp;
    i32 buf_size;
    i32 buf_ptr;
} Nst_StdIn;

extern Nst_StdIn Nst_stdin;

#endif // !Nst_WIN

/**
 * A structure representing a Nest IO file object.
 *
 * @param fp: the pointer to the file, it may not be FILE *
 * @param fd: the file descriptor, -1 if not supported
 * @param encoding: the encoding the file was opended in, NULL when opened in
 * binary mode
 * @param func_set: the functions used to operate the file
 */
NstEXP typedef struct _Nst_IOFileObj {
    Nst_OBJ_HEAD;
    void *fp;
    int fd;
    Nst_CP *encoding;
    Nst_IOFuncSet func_set;
} Nst_IOFileObj;

/* The flags of a IO file. */
NstEXP typedef enum _Nst_IOFileFlag {
    Nst_FLAG_IOFILE_IS_CLOSED = 0b000001,
    Nst_FLAG_IOFILE_IS_BIN    = 0b000010,
    Nst_FLAG_IOFILE_CAN_WRITE = 0b000100,
    Nst_FLAG_IOFILE_CAN_READ  = 0b001000,
    Nst_FLAG_IOFILE_CAN_SEEK  = 0b010000,
    Nst_FLAG_IOFILE_IS_TTY    = 0b100000
} Nst_IOFileFlag;

/**
 * Creates a new Nst_IOFileObj from a C file pointer.
 *
 * @param value: the value of the new object
 * @param bin: if the file is in binary mode
 * @param read: whether the file can be read
 * @param write: whether the file can be written
 * @param encoding: the encoding of the opened file, ignored when bin is true
 *
 * @return The new object on success or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_iof_new(FILE *value, bool bin, bool read,
                                 bool write, Nst_CP *encoding);
/**
 * Creates a new Nst_IOFileObj that is not a C file pointer.
 *
 * @param value: the value of the new object
 * @param bin: if the file is in binary mode
 * @param read: whether the file can be read
 * @param write: whether the file can be written
 * @param seek: whether the file can be seeked
 * @param encoding: the encoding of the opened file, ignored when bin is true
 * @param func_set: custom functions used to read the file
 *
 * @return The new object on success or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_iof_new_fake(void *value, bool bin, bool read,
                                      bool write, bool seek, Nst_CP *encoding,
                                      Nst_IOFuncSet func_set);

/* Destructor of a Nst_IOFileObj. */
NstEXP void NstC _Nst_iofile_destroy(Nst_IOFileObj *obj);

/* Calls the read function of the file, see Nst_IOFile_read_f. */
NstEXP Nst_IOResult NstC Nst_fread(i8 *buf, usize buf_size, usize count,
                                   usize *buf_len, Nst_IOFileObj *f);
/* Calls the write function of the file, see Nst_IOFile_read_f. */
NstEXP Nst_IOResult NstC Nst_fwrite(i8 *buf, usize buf_len, usize *count,
                                    Nst_IOFileObj *f);
/* Calls the flush function of the file, see Nst_IOFile_read_f. */
NstEXP Nst_IOResult NstC Nst_fflush(Nst_IOFileObj *f);
/* Calls the tell function of the file, see Nst_IOFile_read_f. */
NstEXP Nst_IOResult NstC Nst_ftell(Nst_IOFileObj *f, usize *pos);
/* Calls the seek function of the file, see Nst_IOFile_read_f. */
NstEXP Nst_IOResult NstC Nst_fseek(Nst_SeekWhence origin, isize offset,
                                   Nst_IOFileObj *f);
/* Calls the close function of the file, see Nst_IOFile_read_f. */
NstEXP Nst_IOResult NstC Nst_fclose(Nst_IOFileObj *f);

/* Read function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_read(i8 *buf, usize buf_size, usize count,
                                        usize *buf_len, Nst_IOFileObj *f);
/* Write function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_write(i8 *buf, usize buf_len, usize *count,
                                         Nst_IOFileObj *f);
/* Flush function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_flush(Nst_IOFileObj *f);
/* Tell function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_tell(Nst_IOFileObj *f, usize *pos);

NstEXP Nst_IOResult NstC Nst_FILE_seek(Nst_SeekWhence origin, isize offset,
                                        Nst_IOFileObj *f);
/* Close function for standard C file descriptors. */
NstEXP Nst_IOResult NstC Nst_FILE_close(Nst_IOFileObj *f);

/**
 * Gets the details of the Nst_IOError returned by the functions.
 *
 * @brief This function can only be called when the returned Nst_IOResult is
 * either Nst_IO_INVALID_ENCODING or Nst_IO_INVALID_DECODING. If the result is
 * the former ill_encoded_ch will be the code point that could not be encoded,
 * otherwise if the result is the latter ill_encoded_ch will represent the byte
 * that could not be decoded. Similarly encoding_name is the encoding that
 * failed to encode the code point for Nst_IO_INVALID_ENCODING and the name
 * of the one that failed to decode the byte for Nst_IO_INVALID_DECODING.
 *
 * @param ill_encoded_ch: variable filled with the character that failed to
 * encode or decode, may be NULL
 * @param position: variable filled with the position in the file of the
 * encoding error, may be NULL
 * @param encoding_name: variable filled with the name of the encoding, may be
 * NULL
 */
NstEXP void NstC Nst_io_result_get_details(u32 *ill_encoded_ch,
                                           usize *position,
                                           const i8 **encoding_name);
/* Sets the values returned with Nst_io_result_get_details. */
NstEXP void NstC Nst_io_result_set_details(u32 ill_encoded_ch,
                                           usize position,
                                           const i8 *encoding_name);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FILE_H
