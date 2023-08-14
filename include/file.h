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

typedef enum _Nst_SeekWhence {
    Nst_SEEK_SET = 0,
    Nst_SEEK_CUR = 1,
    Nst_SEEK_END = 2
} Nst_SeekWhence;

struct _Nst_IOFileObj;

NstEXP typedef Nst_IOResult (*Nst_IOFile_read_f)(i8 *buf, usize buf_size,
                                                 usize count, usize *buf_len,
                                                 struct _Nst_IOFileObj *f);
NstEXP typedef Nst_IOResult (*Nst_IOFile_write_f)(i8 *buf, usize buf_len,
                                                  usize *count,
                                                  struct _Nst_IOFileObj *f);
NstEXP typedef Nst_IOResult (*Nst_IOFile_flush_f)(struct _Nst_IOFileObj *f);
NstEXP typedef Nst_IOResult (*Nst_IOFile_tell_f)(struct _Nst_IOFileObj *f,
                                                 usize *pos);
NstEXP typedef Nst_IOResult (*Nst_IOFile_seek_f)(Nst_SeekWhence origin,
                                                 isize offset,
                                                 struct _Nst_IOFileObj *f);
NstEXP typedef Nst_IOResult (*Nst_IOFile_close_f)(struct _Nst_IOFileObj *f);

typedef struct _Nst_IOFuncSet {
    Nst_IOFile_read_f read;
    Nst_IOFile_write_f write;
    Nst_IOFile_flush_f flush;
    Nst_IOFile_tell_f tell;
    Nst_IOFile_seek_f seek;
    Nst_IOFile_close_f close;
} Nst_IOFuncSet;

#ifdef Nst_WIN
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

NstEXP Nst_IOResult NstC Nst_fread(i8 *buf, usize buf_size, usize count,
                                   usize *buf_len, Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_fwrite(i8 *buf, usize buf_len, usize *count,
                                    Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_fflush(Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_ftell(Nst_IOFileObj *f, usize *pos);

NstEXP Nst_IOResult NstC Nst_fseek(Nst_SeekWhence origin, isize offset,
                                   Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_fclose(Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_FILE_read(i8 *buf, usize buf_size, usize count,
                                        usize *buf_len, Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_FILE_write(i8 *buf, usize buf_len, usize *count,
                                         Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_FILE_flush(Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_FILE_tell(Nst_IOFileObj *f, usize *pos);

NstEXP Nst_IOResult NstC Nst_FILE_seek(Nst_SeekWhence origin, isize offset,
                                        Nst_IOFileObj *f);

NstEXP Nst_IOResult NstC Nst_FILE_close(Nst_IOFileObj *f);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !FILE_H
