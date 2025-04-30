#ifndef TEST_NEST_H
#define TEST_NEST_H

#include "nest.h"

#if defined(Nst_GCC) || defined(Nst_CLANG)
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-label"
#elif defined(Nst_MSVC)
#pragma warning(disable: 4102)
#endif

#define run_test(test) run_test_(test, #test);

#define ENTER_TEST TestResult test_result__ = TEST_SUCCESS
#define EXIT_TEST test_exit: return test_result__

// To this function are passed the va_args of fail_if and crit_fail_if
// By default they will free pointers as Nest objects with Nst_dec_ref,
// Pass F_P to begin freeing pointers with Nst_free
// Pass F_O to start freeing objects again
// Pass F_C followed by a function of type void (*)(void *), the pointers
// will then be passed to that function
// Any NULL pointer is automatically skipped
// F_END will end the series even with pointers still in the series, it is
// added automatically after all aruments when calling fail_if or crit_fail_if
// F_ENDIF will stop if the given pointer is NULL.

// Pass to fail_if to begin freeing objects (with Nst_dec_ref)
#define F_O ((void *) 2)
// Pass to fail_if to begin freeing pointers (with Nst_free)
#define F_P ((void *) 3)
// Pass to fail_if to begin freeing custom pointers, the first parameter after
// this will be the function used
#define F_C ((void *) 4)

#define F_END ((void *) 1)

#define F_ENDIF(ptr) (void *)((usize)(ptr) * (usize)F_END))

#define fail_if(cond, ...)                                                    \
    fail_if_((cond), &test_result__, __LINE__, ## __VA_ARGS__, F_END)

#define crit_fail_if(cond, ...) do {                                          \
    if (crit_fail_if_(                                                        \
            (cond),                                                           \
            &test_result__,                                                   \
            __LINE__, ## __VA_ARGS__,                                         \
            F_END))                                                           \
    {                                                                         \
        goto test_exit;                                                       \
    }} while (0)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef enum _TestResult {
    TEST_SUCCESS,
    TEST_FAILURE,
    TEST_CRITICAL_FAILURE,
    TEST_NOT_IMPL
} TestResult;

typedef TestResult (*Test)(void);

void test_init(void);
void run_test_(Test test, const char *name);
i32 tests_failed_count(void);

// Begin capturing the output of stdout and stderr. The return value is false
// on error.
bool capture_output_begin(void);
// Finish capturing the output of stdout and stderr, the return value is valid
// until the next call to `capture_output_begin`
// out_lenth is set to the length of the buffer, can be NULL. The buffer is
// NUL-terminated or NULL on error.
const char *capture_output_end(usize *out_length);

bool fail_if_(bool cond, TestResult *result, int line, ...);
bool crit_fail_if_(bool cond, TestResult *result, int line, ...);
// Check if strings are different, if str1 is NULL it returns true.
bool str_neq(u8 *str1, const char *str2);
// Object to bool, takes a reference from the object
bool ref_obj_to_bool(Nst_Obj *obj);

// argv_parser.h

TestResult test_cl_args_parse(void);

#ifdef Nst_MSVC
TestResult test_wargv_to_argv(void);
#endif

// dyn_array.h

TestResult test_da_init(void);
TestResult test_da_init_copy(void);
TestResult test_da_reserve(void);
TestResult test_da_append(void);
TestResult test_da_remove_swap(void);
TestResult test_da_remove_shift(void);
TestResult test_da_at(void);
TestResult test_da_clear(void);

// encoding.h

TestResult test_check_bytes(void);
TestResult test_to_utf32(void);
TestResult test_from_utf32(void);
TestResult test_utf16_to_utf8(void);
TestResult test_encoding_translate(void);
TestResult test_encoding_check(void);
TestResult test_encoding_char_len(void);
TestResult test_encoding_utf8_char_len(void);
TestResult test_char_to_wchar_t(void);
TestResult test_wchar_t_to_char(void);
TestResult test_is_valid_cp(void);
TestResult test_is_non_character(void);
TestResult test_check_bom(void);
TestResult test_encoding_detect(void);
TestResult test_encoding_from_name(void);
TestResult test_encoding_to_single_byte(void);

// error.h

TestResult test_tb_add_pos(void);
TestResult test_error_set(void);
TestResult test_error_setc(void);
TestResult test_error_setf(void);
TestResult test_error_add_pos(void);
TestResult test_error_occurred(void);
TestResult test_error_get(void);
TestResult test_error_clear(void);

// file.h

TestResult test_FILE_read(void);
TestResult test_FILE_write(void);
TestResult test_FILE_flush(void);
TestResult test_FILE_tell(void);
TestResult test_FILE_seek(void);
TestResult test_FILE_close(void);
TestResult test_io_result_details(void);
TestResult test_fopen_unicode(void);

// format.h

TestResult test_sprintf(void);
TestResult test_fmt(void);
TestResult test_fmt_objs(void);
TestResult test_repr(void);

// function.h

TestResult test_func_set_vt(void);

// hash.h

TestResult test_obj_hash(void);

// iter.h

TestResult test_iter_start(void);
TestResult test_iter_get_val(void);

// lib_import.h

TestResult test_extract_args(void);

// llist.h

TestResult test_llist_push(void);
TestResult test_llist_append(void);
TestResult test_llist_insert(void);
TestResult test_llist_pop(void);
TestResult test_llist_peek_front(void);
TestResult test_llist_peek_back(void);
TestResult test_llist_push_llnode(void);
TestResult test_llist_append_llnode(void);
TestResult test_llist_pop_llnode(void);
TestResult test_llist_empty(void);
TestResult test_llist_move_nodes(void);

// map.h

TestResult test_map_new(void);
TestResult test_map_copy(void);
TestResult test_map_set(void);
TestResult test_map_get(void);
TestResult test_map_drop(void);
TestResult test_map_set_str(void);
TestResult test_map_get_str(void);
TestResult test_map_drop_str(void);
TestResult test_map_next(void);
TestResult test_map_prev(void);

// mem.h

TestResult test_malloc(void);
TestResult test_calloc(void);
TestResult test_realloc(void);
TestResult test_crealloc(void);
TestResult test_memset(void);

// sequence.h

TestResult test_seq_new(void);
TestResult test_seq_create(void);
TestResult test_seq_create_c(void);
TestResult test_seq_set(void);
TestResult test_seq_get(void);
TestResult test_seq_copy(void);
TestResult test_vector_append(void);
TestResult test_vector_remove(void);
TestResult test_vector_pop(void);

// simple_types.h

TestResult test_number_to_u8(void);
TestResult test_number_to_int(void);
TestResult test_number_to_i32(void);
TestResult test_number_to_i64(void);
TestResult test_number_to_f32(void);
TestResult test_number_to_f64(void);
TestResult test_obj_to_bool(void);

// str.h

TestResult test_str_new_c_raw(void);
TestResult test_str_new_c(void);
TestResult test_str_new(void);
TestResult test_str_new_allocated(void);
TestResult test_str_new_len(void);
TestResult test_str_from_sv(void);
TestResult test_str_from_sb(void);
TestResult test_str_copy(void);
TestResult test_str_repr(void);
TestResult test_str_get_obj(void);
TestResult test_str_get(void);
TestResult test_str_next(void);
TestResult test_str_next_obj(void);
TestResult test_str_next_utf32(void);
TestResult test_str_next_utf8(void);
TestResult test_str_parse_int(void);
TestResult test_str_parse_byte(void);
TestResult test_str_parse_real(void);
TestResult test_str_compare(void);

// str_builder.h

TestResult test_sb_init(void);
TestResult test_sb_reserve(void);
TestResult test_sb_push(void);
TestResult test_sb_push_sv(void);
TestResult test_sb_push_str(void);
TestResult test_sb_push_c(void);
TestResult test_sb_push_cps(void);
TestResult test_sb_push_char(void);

// str_view.h

TestResult test_sv_new(void);
TestResult test_sv_new_c(void);
TestResult test_sv_from_str(void);
TestResult test_sv_from_sb(void);
TestResult test_sv_next(void);
TestResult test_sv_prev(void);
TestResult test_sv_parse_int(void);
TestResult test_sv_parse_byte(void);
TestResult test_sv_parse_real(void);
TestResult test_sv_compare(void);
TestResult test_sv_lfind(void);
TestResult test_sv_rfind(void);
TestResult test_sv_ltok(void);
TestResult test_sv_rtok(void);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TEST_NEST_H
