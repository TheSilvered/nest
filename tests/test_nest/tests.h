#ifndef TEST_NEST_H
#define TEST_NEST_H

#include "nest.h"

#if defined(Nst_GCC) || defined(Nst_CLANG)
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-label"
#elif defined(Nst_MSVC)
#pragma warning(disable: 4102)
#endif

#define test_run(test) test_run_(test, #test);

#define TEST_ENTER TestResult test_result__ = TEST_SUCCESS
#define TEST_EXIT test_exit: return test_result__

// Assert that `cond` is true or log a failure
#define test_assert(cond) test_assert_((cond), &test_result__, __LINE__)

// Usage: test_with(cond) { if_cond_is_true; }
#define test_with(cond) if (test_assert_((cond), &test_result__, __LINE__))

// Assert that `cond` is true or log a failure and exit the test
#define test_assert_or_exit(cond, on_failure) do {                            \
    if (!test_assert_or_exit_(                                                \
            (cond),                                                           \
            &test_result__,                                                   \
            __LINE__))                                                        \
    {                                                                         \
        {on_failure;}                                                         \
        goto test_exit;                                                       \
    }} while (0)

// Assert that `cond` is true or log a failure and exit the program
#define test_assert_or_abort(cond, on_failure) do {                           \
    if (!test_assert_or_abort_(                                               \
            (cond),                                                           \
            &test_result__,                                                   \
            __LINE__))                                                        \
    {                                                                         \
        {on_failure;}                                                         \
        goto test_exit;                                                       \
    }} while (0)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef enum _TestResult {
    TEST_SUCCESS,
    TEST_FAILURE,
    TEST_CRITICAL_FAILURE,
    TEST_CRITICAL_ABORT,
    TEST_NOT_IMPL
} TestResult;

typedef TestResult (*Test)(void);

// Initialize tests
void test_init(void);
// Get the number of failed tests
i32 tests_failed_count(void);

void test_run_(Test test, const char *name);

// Begin capturing the output of stdout and stderr. The return value is false
// on error.
bool test_capture_begin(void);
// Finish capturing the output of stdout and stderr, the return value is valid
// until the next call to `capture_output_begin`
// out_lenth is set to the length of the buffer, can be NULL. The buffer is
// NUL-terminated or NULL on error.
const char *test_capture_end(usize *out_length);

bool test_assert_(bool cond, TestResult *result, int line);
bool test_assert_or_exit_(bool cond, TestResult *result, int line);
// Check if strings are different, if str1 is NULL it returns true.
bool str_eq(u8 *str1, const char *str2);
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
TestResult test_da_get(void);
TestResult test_da_set(void);
TestResult test_da_clear(void);
TestResult test_pa_init(void);
TestResult test_pa_init_copy(void);
TestResult test_pa_reserve(void);
TestResult test_pa_append(void);
TestResult test_pa_remove_swap(void);
TestResult test_pa_remove_shift(void);
TestResult test_pa_get(void);
TestResult test_pa_set(void);
TestResult test_pa_clear(void);

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
TestResult test_cp_is_valid(void);
TestResult test_cp_is_non_character(void);
TestResult test_check_bom(void);
TestResult test_encoding_detect(void);
TestResult test_encoding_from_name(void);
TestResult test_encoding_to_single_byte(void);

// error.h

TestResult test_span_new(void);
TestResult test_span_from_pos(void);
TestResult test_span_join(void);
TestResult test_span_extend(void);
TestResult test_span_start(void);
TestResult test_span_end(void);
TestResult test_error_set(void);
TestResult test_error_setc(void);
TestResult test_error_setf(void);
TestResult test_error_failed_alloc(void);
TestResult test_error_add_span(void);
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

TestResult test_iter_start_func(void);
TestResult test_iter_next_func(void);
TestResult test_iter_value(void);
TestResult test_iter_start(void);
TestResult test_iter_get_val(void);
TestResult test_iter_range_new(void);
TestResult test_iter_seq_new(void);
TestResult test_iter_str_new(void);
TestResult test_iter_map_new(void);

// lib_import.h

TestResult test_extract_args(void);
TestResult test_obj_custom(void);
TestResult test_obj_custom_ex(void);
TestResult test_obj_custom_data(void);

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
TestResult test_map_len(void);
TestResult test_map_cap(void);
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
TestResult test_seq_from_objs(void);
TestResult test_seq_create(void);
TestResult test_seq_create_c(void);
TestResult test_seq_len(void);
TestResult test_vector_cap(void);
TestResult test_seq_objs(void);
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

// source_loader.h

TestResult test_source_from_sv(void);

// str.h

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
