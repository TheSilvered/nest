#ifndef TEST_NEST_H
#define TEST_NEST_H

#include "nest.h"

#define RUN_TEST(test) run_test(test, #test);

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef enum _TestResult {
    TEST_SUCCESS,
    TEST_FAILURE,
    TEST_CRITICAL_FAILURE,
    TEST_NOT_IMPL,
    TEST_NEST_ERROR
} TestResult;

typedef TestResult (*Test)();

void test_init();
void run_test(Test test, const i8 *name);

// argv_parser.h

TestResult test_parse_args();

#ifdef Nst_WIN
TestResult test_wargv_to_argv();
#endif

// encoding.h

TestResult test_check_bytes();
TestResult test_to_utf32();
TestResult test_from_utf32();
TestResult test_utf16_to_utf8();
TestResult test_translate_cp();
TestResult test_check_string_cp();
TestResult test_string_char_len();
TestResult test_string_utf8_char_len();
TestResult test_char_to_wchar_t();
TestResult test_wchar_t_to_char();
TestResult test_is_valid_cp();
TestResult test_is_non_character();
TestResult test_check_bom();
TestResult test_detect_encoding();
TestResult test_encoding_from_name();
TestResult test_single_byte_cp();

// error.h

TestResult test_set_error();
TestResult test_set_error_c();
TestResult test_error_occurred();
TestResult test_error_get();
TestResult test_error_clear();
TestResult test_set_error_internal();

// file.h

TestResult test_FILE_read();
TestResult test_FILE_write();
TestResult test_FILE_flush();
TestResult test_FILE_tell();
TestResult test_FILE_seek();
TestResult test_FILE_close();
TestResult test_io_result_details();
TestResult test_fopen_unicode();

// format.h

TestResult test_sprintf();
TestResult test_fmt();

// function.h

TestResult test_func_set_vt();

// hash.h

TestResult test_obj_hash();

// iter.h

TestResult test_iter_start();
TestResult test_iter_get_val();

// lib_import.h

TestResult test_extract_args();

// llist.h

TestResult test_llist_push();
TestResult test_llist_append();
TestResult test_llist_insert();
TestResult test_llist_pop();
TestResult test_llist_peek_front();
TestResult test_llist_peek_back();
TestResult test_llist_push_llnode();
TestResult test_llist_append_llnode();
TestResult test_llist_pop_llnode();
TestResult test_llist_empty();
TestResult test_llist_move_nodes();

// map.h

TestResult test_map_set();
TestResult test_map_get();
TestResult test_map_drop();
TestResult test_map_copy();
TestResult test_map_get_next_idx();
TestResult test_map_get_prev_idx();
TestResult test_map_set_str();
TestResult test_map_get_str();
TestResult test_map_drop_str();

// mem.h

TestResult test_alloc();
TestResult test_sbuffer_expand_by();
TestResult test_sbuffer_expand_to();
TestResult test_sbuffer_fit();
TestResult test_sbuffer_append();
TestResult test_sbuffer_pop();
TestResult test_sbuffer_at();
TestResult test_sbuffer_shrink_auto();
TestResult test_sbuffer_copy();
TestResult test_buffer_expand_by();
TestResult test_buffer_expand_to();
TestResult test_buffer_fit();
TestResult test_buffer_append();
TestResult test_buffer_append_c_str();
TestResult test_buffer_append_str();
TestResult test_buffer_append_char();
TestResult test_buffer_to_string();
TestResult test_buffer_copy();

// sequence.h

TestResult test_seq_set();
TestResult test_seq_get();
TestResult test_seq_copy();
TestResult test_vector_append();
TestResult test_vector_remove();
TestResult test_vector_pop();
TestResult test_array_vector_create();
TestResult test_array_vector_create_c();

// simple_types.h

TestResult test_number_to_u8();
TestResult test_number_to_int();
TestResult test_number_to_i32();
TestResult test_number_to_i64();
TestResult test_number_to_f32();
TestResult test_number_to_f64();
TestResult test_obj_to_bool();

// str.h

TestResult test_string_copy();
TestResult test_string_repr();
TestResult test_string_get();
TestResult test_string_next_ch();
TestResult test_string_new_c_raw();
TestResult test_string_new_c();
TestResult test_string_new();
TestResult test_string_parse_int();
TestResult test_string_parse_byte();
TestResult test_string_parse_real();
TestResult test_string_compare();
TestResult test_string_find();
TestResult test_string_rfind();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TEST_NEST_H
