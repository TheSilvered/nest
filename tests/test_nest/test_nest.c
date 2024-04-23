#include <stdlib.h>

#include "test_nest.h"

const i8 *RED = "\x1b[31m";
const i8 *GREEN = "\x1b[32m";
const i8 *YELLOW = "\x1b[33m";
const i8 *RESET = "\x1b[0m";

void test_init()
{
    if (!Nst_supports_color()) {
        RED = "";
        GREEN = "";
        YELLOW = "";
    }
}

void run_test(Test test, const i8 *test_name)
{
    TestResult result = test();

    switch (result) {
    case TEST_SUCCESS:
        Nst_printf(
            "%sTest '%s' passed.%s\n",
            GREEN, test_name, RESET);
        break;
    case TEST_FAILURE:
        Nst_printf(
            "%sTest '%s' failed.%s\n",
            RED, test_name, RESET);
        break;
    case TEST_NOT_IMPL:
        Nst_printf(
            "%sTest '%s' not implemented.%s\n",
            YELLOW, test_name, RESET);
        break;
    case TEST_CRITICAL_FAILURE:
        Nst_printf(
            "%sTest '%s' failed.%s Stopping execution...\n",
            YELLOW, test_name, RESET);
        exit(1);
    }
}

// argv_parser.h

TestResult test_parse_args()
{
    return TEST_NOT_IMPL;
}

#ifdef Nst_WIN

TestResult test_wargv_to_argv()
{
    return TEST_NOT_IMPL;
}

#endif

// encoding.h

TestResult test_check_bytes()
{
    return TEST_NOT_IMPL;
}

TestResult test_to_utf32()
{
    return TEST_NOT_IMPL;
}

TestResult test_from_utf32()
{
    return TEST_NOT_IMPL;
}

TestResult test_utf16_to_utf8()
{
    return TEST_NOT_IMPL;
}

TestResult test_translate_cp()
{
    return TEST_NOT_IMPL;
}

TestResult test_check_string_cp()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_char_len()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_utf8_char_len()
{
    return TEST_NOT_IMPL;
}

TestResult test_char_to_wchar_t()
{
    return TEST_NOT_IMPL;
}

TestResult test_wchar_t_to_char()
{
    return TEST_NOT_IMPL;
}

TestResult test_is_valid_cp()
{
    return TEST_NOT_IMPL;
}

TestResult test_is_non_character()
{
    return TEST_NOT_IMPL;
}

TestResult test_check_bom()
{
    return TEST_NOT_IMPL;
}

TestResult test_detect_encoding()
{
    return TEST_NOT_IMPL;
}

TestResult test_encoding_from_name()
{
    return TEST_NOT_IMPL;
}

TestResult test_single_byte_cp()
{
    return TEST_NOT_IMPL;
}

// error.h

TestResult test_set_error()
{
    return TEST_NOT_IMPL;
}

TestResult test_set_error_c()
{
    return TEST_NOT_IMPL;
}

TestResult test_error_occurred()
{
    return TEST_NOT_IMPL;
}

TestResult test_error_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_error_clear()
{
    return TEST_NOT_IMPL;
}

TestResult test_set_error_internal()
{
    return TEST_NOT_IMPL;
}

// file.h

TestResult test_FILE_read()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_write()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_flush()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_tell()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_seek()
{
    return TEST_NOT_IMPL;
}

TestResult test_FILE_close()
{
    return TEST_NOT_IMPL;
}

TestResult test_io_result_details()
{
    return TEST_NOT_IMPL;
}

TestResult test_fopen_unicode()
{
    return TEST_NOT_IMPL;
}

// format.h

TestResult test_sprintf()
{
    return TEST_NOT_IMPL;
}

TestResult test_fmt()
{
    return TEST_NOT_IMPL;
}

// function.h

TestResult test_func_set_vt()
{
    return TEST_NOT_IMPL;
}

// hash.h

TestResult test_obj_hash()
{
    return TEST_NOT_IMPL;
}

// iter.h

TestResult test_iter_start()
{
    return TEST_NOT_IMPL;
}

TestResult test_iter_get_val()
{
    return TEST_NOT_IMPL;
}

// lib_import.h

TestResult test_extract_arg_values()
{
    return TEST_NOT_IMPL;
}

// llist.h

TestResult test_llist_push()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_append()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_insert()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_pop()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_peek_front()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_peek_back()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_push_llnode()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_append_llnode()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_pop_llnode()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_empty()
{
    return TEST_NOT_IMPL;
}

TestResult test_llist_move_nodes()
{
    return TEST_NOT_IMPL;
}

// map.h

TestResult test_map_set()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_drop()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_copy()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_get_next_idx()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_get_prev_idx()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_set_str()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_get_str()
{
    return TEST_NOT_IMPL;
}

TestResult test_map_drop_str()
{
    return TEST_NOT_IMPL;
}

// mem.h

TestResult test_alloc()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_expand_by()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_expand_to()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_fit()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_append()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_copy()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_expand_by()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_expand_to()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_fit()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_append()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_append_c_str()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_append_str()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_append_char()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_to_string()
{
    return TEST_NOT_IMPL;
}

TestResult test_buffer_copy()
{
    return TEST_NOT_IMPL;
}

// sequence.h

TestResult test_seq_set()
{
    return TEST_NOT_IMPL;
}

TestResult test_seq_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_seq_copy()
{
    return TEST_NOT_IMPL;
}

TestResult test_vector_append()
{
    return TEST_NOT_IMPL;
}

TestResult test_vector_remove()
{
    return TEST_NOT_IMPL;
}

TestResult test_vector_pop()
{
    return TEST_NOT_IMPL;
}

TestResult test_array_vector_create()
{
    return TEST_NOT_IMPL;
}

TestResult test_array_vector_create_c()
{
    return TEST_NOT_IMPL;
}

// simple_types.h

TestResult test_number_to_u8()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_int()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_i32()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_i64()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_f32()
{
    return TEST_NOT_IMPL;
}

TestResult test_number_to_f64()
{
    return TEST_NOT_IMPL;
}

TestResult test_obj_to_bool()
{
    return TEST_NOT_IMPL;
}

// str.h

TestResult test_string_copy()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_repr()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_get()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_next_ch()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_new_c_raw()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_new_c()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_new()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_parse_int()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_parse_byte()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_parse_real()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_compare()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_find()
{
    return TEST_NOT_IMPL;
}

TestResult test_string_rfind()
{
    return TEST_NOT_IMPL;
}
