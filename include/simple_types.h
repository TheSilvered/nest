/**
 * @file simple_types.h
 *
 * @brief Interface for Nst_IntObj, Nst_RealObj, Nst_ByteObj, Nst_BoolObj and
 * Nst_IOFileObj
 *
 * @author TheSilvered
 */

#ifndef SYMPLE_TYPES_H
#define SYMPLE_TYPES_H

#include <stdio.h>
#include "error.h"

/* Casts ptr to a Nst_IntObj * and extracts the value field. */
#define AS_INT(ptr)  (((Nst_IntObj  *)(ptr))->value)
/* Casts ptr to a Nst_RealObj * and extracts the value field. */
#define AS_REAL(ptr) (((Nst_RealObj *)(ptr))->value)
/* Casts ptr to a Nst_ByteObj * and extracts the value field. */
#define AS_BYTE(ptr) (((Nst_ByteObj *)(ptr))->value)
/* Casts ptr to a Nst_BoolObj * and extracts the value field. */
#define AS_BOOL(ptr) (((Nst_BoolObj *)(ptr))->value)
/* Casts ptr to a Nst_IOFileObj *. */
#define IOFILE(ptr) ((Nst_IOFileObj *)(ptr))

/* Checks if f is closed. */
#define Nst_IOF_IS_CLOSED(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_IS_CLOSED)
/* Checks if f was opened in binary mode. */
#define Nst_IOF_IS_BIN(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_IS_BIN)
/* Checks if f can be written. */
#define Nst_IOF_CAN_WRITE(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_CAN_WRITE)
/* Checks if f can be read. */
#define Nst_IOF_CAN_READ(f) Nst_FLAG_HAS(f, Nst_FLAG_IOFILE_CAN_READ)

/* Alias for _Nst_number_to_u8 that casts number to Nst_Obj *. */
#define Nst_number_to_u8(number) _Nst_number_to_u8(OBJ(number))
/* Alias for _Nst_number_to_int that casts number to Nst_Obj *. */
#define Nst_number_to_int(number) _Nst_number_to_int(OBJ(number))
/* Alias for _Nst_number_to_i32 that casts number to Nst_Obj *. */
#define Nst_number_to_i32(number) _Nst_number_to_i32(OBJ(number))
/* Alias for _Nst_number_to_i64 that casts number to Nst_Obj *. */
#define Nst_number_to_i64(number) _Nst_number_to_i64(OBJ(number))
/* Alias for _Nst_number_to_f32 that casts number to Nst_Obj *. */
#define Nst_number_to_f32(number) _Nst_number_to_f32(OBJ(number))
/* Alias for _Nst_number_to_f64 that casts number to Nst_Obj *. */
#define Nst_number_to_f64(number) _Nst_number_to_f64(OBJ(number))
/* Alias for _Nst_obj_to_bool that casts obj to Nst_Obj *. */
#define Nst_obj_to_bool(obj) _Nst_obj_to_bool(OBJ(obj))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A structure representing a Nest integer object.
 *
 * @param value: the value of the integer
 */
NstEXP typedef struct _Nst_IntObj {
    Nst_OBJ_HEAD;
    i64 value;
} Nst_IntObj;

/**
 * A structure representing a Nest real number (floating-point) object.
 *
 * @param value: the value of the real number
 */
NstEXP typedef struct _Nst_RealObj {
    Nst_OBJ_HEAD;
    f64 value;
} Nst_RealObj;

/**
 * A structure representing a Nest boolean object.
 *
 * @param value: the value of the boolean
 */
NstEXP typedef struct _Nst_BoolObj {
    Nst_OBJ_HEAD;
    bool value;
} Nst_BoolObj;

/**
 * A structure representing a Nest byte object.
 *
 * @param value: the value of the byte
 */
NstEXP typedef struct _Nst_ByteObj {
    Nst_OBJ_HEAD;
    u8 value;
} Nst_ByteObj;

/* The type of the value of a Nst_IOFileObj. */
NstEXP typedef FILE *Nst_IOFile;
/* The type of the read function for a Nst_IOFileObj. */
NstEXP typedef usize (*Nst_IOFile_read_f) (void *buf, usize size, usize count,
                                           void *f_value);
/* The type of the write function for a Nst_IOFileObj. */
NstEXP typedef usize (*Nst_IOFile_write_f) (void *buf, usize size, usize count,
                                            void *f_value);
/* The type of the flush function for a Nst_IOFileObj. */
NstEXP typedef int (*Nst_IOFile_flush_f) (void *f_value);
/* The type of the tell function for a Nst_IOFileObj. */
NstEXP typedef i32 (*Nst_IOFile_tell_f) (void *f_value);
/* The type of the seek function for a Nst_IOFileObj. */
NstEXP typedef int (*Nst_IOFile_seek_f) (void *f_value, i32 offset, int origin);
/* The type of the close function for a Nst_IOFileObj. */
NstEXP typedef int (*Nst_IOFile_close_f) (void *f_value);

/**
 * A structure representing a Nest IO file object.
 *
 * @param value: the file iself, usually a file handle
 * @param read_f: the read function for the file
 * @param write_f: the write function for the file
 * @param flush_f: the flush function for the file
 * @param tell_f: the tell function for the file
 * @param seek_f: the seek function for the file
 * @param close_f: the close function for the file
 */
NstEXP typedef struct _Nst_IOFileObj {
    Nst_OBJ_HEAD;
    Nst_IOFile value;
    Nst_IOFile_read_f  read_f;
    Nst_IOFile_write_f write_f;
    Nst_IOFile_flush_f flush_f;
    Nst_IOFile_tell_f  tell_f;
    Nst_IOFile_seek_f  seek_f;
    Nst_IOFile_close_f close_f;
} Nst_IOFileObj;

/* The flags of a IO file. */
NstEXP typedef enum _Nst_IOFileFlag {
    Nst_FLAG_IOFILE_IS_CLOSED = 0b0001,
    Nst_FLAG_IOFILE_IS_BIN    = 0b0010,
    Nst_FLAG_IOFILE_CAN_WRITE = 0b0100,
    Nst_FLAG_IOFILE_CAN_READ  = 0b1000
} Nst_IOFileFlag;

/**
 * Creates a new Nst_IntObj.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_int_new(i64 value);
/**
* Creates a new Nst_RealObj.
*
* @param value: the value of the new object
*
* @return The new object on success or NULL on failure. The error is set.
*/
NstEXP Nst_Obj *NstC Nst_real_new(f64 value);
/**
 * Creates a new Nst_BoolObj.
 *
 * @brief This function should never be called, to get the true and false
 * objects use Nst_true() and Nst_false() instead. Note that these functions
 * do not return a new reference to the returned objects.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_bool_new(bool value);
/**
 * Creates a new Nst_ByteObj.
 *
 * @param value: the value of the new object
 *
 * @return The new object on success or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_byte_new(u8 value);
// Creates a new IOFile object, bin: is opened in binary format,
// read: supports reading, write: supports writing
/**
 * Creates a new Nst_IOFileObj from a C file pointer.
 *
 * @param value: the value of the new object
 * @param bin: if the file is in binary mode
 * @param read: whether the file can be read
 * @param write: whether the file can be written
 *
 * @return The new object on success or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_iof_new(Nst_IOFile value, bool bin, bool read,
                                 bool write);
/**
 * Creates a new Nst_IOFileObj that is not a C file pointer.
 *
 * @param value: the value of the new object
 * @param bin: if the file is in binary mode
 * @param read: whether the file can be read
 * @param write: whether the file can be written
 * @param read_f: the custom read function for the file
 * @param write_f: the custom write function for the file
 * @param flush_f: the custom flush function for the file
 * @param tell_f: the custom tell function for the file
 * @param seek_f: the custom seek function for the file
 * @param close_f: the custom close function for the file
 *
 * @return The new object on success or NULL on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_iof_new_fake(void *value, bool bin, bool read,
                                      bool write, Nst_IOFile_read_f read_f,
                                      Nst_IOFile_write_f write_f,
                                      Nst_IOFile_flush_f flush_f,
                                      Nst_IOFile_tell_f tell_f,
                                      Nst_IOFile_seek_f seek_f,
                                      Nst_IOFile_close_f close_f);

/* Destructor of a Nst_IOFileObj. */
NstEXP void NstC _Nst_iofile_destroy(Nst_IOFileObj *obj);

/**
 * Calls the read_f function of a Nest file.
 *
 * @brief If the file is closed or cannot be read, the function returns -1.
 *
 * @param buf: the buffer where to put the contents read
 * @param size: the size of one element to read
 * @param count: the number of elements to read
 * @param f: the file to read from
 *
 * @return The number of elements read, it may not match count. No error is
 * set.
 */
NstEXP isize NstC Nst_fread(void *buf, usize size, usize count,
                            Nst_IOFileObj *f);
/**
 * Calls the write_f function of a Nest file.
 *
 * @brief If the file is closed or cannot be written, the function returns -1.
 *
 * @param buf: the buffer where to put the contents read
 * @param size: the size of one element to read
 * @param count: the number of elements to read
 * @param f: the file to read from
 *
 * @return The number of elements written, it may not match count. No error is
 * set.
 */
NstEXP isize NstC Nst_fwrite(void *buf, usize size, usize count,
                             Nst_IOFileObj *f);
/**
 * Calls the flush_f function of a Nest file.
 *
 * @param f: the file to flush
 *
 * @return 0 on success and -1 on failure. No error is set.
 */
NstEXP i32 NstC Nst_fflush(Nst_IOFileObj *f);
/**
 * Calls the flush_f function of a Nest file.
 *
 * @param f: the file to get the file indicator position of
 *
 * @return The position in bytes from the start of the file of the file
 * indicator or -1 on failure. No error is set.
 */
NstEXP i32 NstC Nst_ftell(Nst_IOFileObj *f);
/**
 * Calls the seek_f function of a Nest file.
 *
 * @param f: the file to set the indicator position of
 * @param offset: the offset in bytes from the origin
 * @param origin: the start of the offset bytes
 *
 * @return 0 on success and -1 of failure. No error is set.
 */
NstEXP i32 NstC Nst_fseek(Nst_IOFileObj *f, i32 offset, i32 origin);
/**
 * Calls the close_f function of a Nest file.
 *
 * @brief Do not call the close_f function manually since this function
 * correctly sets the Nst_FLAG_IOFILE_IS_CLOSED flag.
 *
 * @param f: the file to be closed
 *
 * @return 0 on success and -1 on failure. No error is set.
 */
NstEXP i32 NstC Nst_fclose(Nst_IOFileObj *f);

/**
 * Converts the value of a numeric object (Nst_IntObj, Nst_RealObj,
 * Nst_ByteObj) to a u8.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object 0 is
 * returned. No error is set.
 */
NstEXP u8  NstC _Nst_number_to_u8(Nst_Obj *number);
/**
 * Converts the value of a numeric object (Nst_IntObj, Nst_RealObj,
 * Nst_ByteObj) to an int.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object 0 is
 * returned. No error is set.
 */
NstEXP int NstC _Nst_number_to_int(Nst_Obj *number);
/**
 * Converts the value of a numeric object (Nst_IntObj, Nst_RealObj,
 * Nst_ByteObj) to an i32.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object 0 is
 * returned. No error is set.
 */
NstEXP i32 NstC _Nst_number_to_i32(Nst_Obj *number);
/**
 * Converts the value of a numeric object (Nst_IntObj, Nst_RealObj,
 * Nst_ByteObj) to an i64.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object 0 is
 * returned. No error is set.
 */
NstEXP i64 NstC _Nst_number_to_i64(Nst_Obj *number);
/**
 * Converts the value of a numeric object (Nst_IntObj, Nst_RealObj,
 * Nst_ByteObj) to an f32.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object 0 is
 * returned. No error is set.
 */
NstEXP f32 NstC _Nst_number_to_f32(Nst_Obj *number);
/**
 * Converts the value of a numeric object (Nst_IntObj, Nst_RealObj,
 * Nst_ByteObj) to an f64.
 *
 * @param number: the object to convert the value of
 *
 * @return The converted number. If number is not a numeric object 0 is
 * returned. No error is set.
 */
NstEXP f64 NstC _Nst_number_to_f64(Nst_Obj *number);
/**
 * @brief Converts any object to a boolean. Exactly the same as casting the
 * object to Nst_type()->Bool and then checking if the result matches
 * Nst_true();
 */
NstEXP bool NstC _Nst_obj_to_bool(Nst_Obj *obj);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SYMPLE_TYPES_H
