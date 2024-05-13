#include <stdlib.h>

#include "test_nest.h"

#define FAIL_IF(cond) do {                                                    \
    if (cond) {                                                               \
        Nst_printf("%s  Failure on line %i%s\n", RED, __LINE__, RESET);       \
        result = TEST_FAILURE;                                                \
    }} while (0)                                                              \

#define FAIL_EXIT_FUNC_IF(cond) do {                                          \
    if (cond) {                                                               \
        Nst_printf("%s  Failure on line %i%s\n", RED, __LINE__, RESET);       \
        result = TEST_FAILURE;                                                \
        goto failure;                                                         \
    }} while (0)

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
        Nst_quit();
        exit(1);
    case TEST_NEST_ERROR:
        Nst_printf(
            "%sDuring test '%s' a Nest error occurred.%s\n",
            YELLOW, test_name, RESET);
        Nst_print_traceback(Nst_error_get());
        Nst_quit();
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
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_push(&llist, (void *)1, false);
    FAIL_IF(llist.len != 1);
    Nst_llist_push(&llist, (void *)2, false);
    FAIL_IF(llist.len != 2);
    Nst_llist_push(&llist, (void *)3, false);
    FAIL_IF(llist.len != 3);

    FAIL_IF(llist.head->value != (void *)3);
    FAIL_IF(llist.head->next->value != (void *)2);
    FAIL_IF(llist.tail->value != (void *)1);
    FAIL_IF(llist.head->next->next != llist.tail);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_append()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    FAIL_IF(llist.len != 1);
    Nst_llist_append(&llist, (void *)2, false);
    FAIL_IF(llist.len != 2);
    Nst_llist_append(&llist, (void *)3, false);
    FAIL_IF(llist.len != 3);

    FAIL_IF(llist.head->value != (void *)1);
    FAIL_IF(llist.head->next->value != (void *)2);
    FAIL_IF(llist.tail->value != (void *)3);
    FAIL_IF(llist.head->next->next != llist.tail);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_insert()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);

    Nst_llist_insert(&llist, (void *)3, false, llist.head);
    FAIL_IF(llist.head->next->value != (void *)3);
    Nst_llist_insert(&llist, (void *)4, false, llist.tail);
    FAIL_IF(llist.tail->value != (void *)4);
    Nst_llist_insert(&llist, (void *)5, false, NULL);
    FAIL_IF(llist.head->value != (void *)5);

    FAIL_IF(llist.len != 5);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_pop()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    FAIL_IF(Nst_llist_pop(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_pop(&llist);
    FAIL_IF(llist.len != 1);
    FAIL_IF(value != (void *)1);
    Nst_llist_pop(&llist);
    FAIL_IF(llist.head != NULL);
    FAIL_IF(llist.tail != NULL);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_peek_front()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    FAIL_IF(Nst_llist_peek_front(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_peek_front(&llist);
    FAIL_IF(llist.len != 2);
    FAIL_IF(value != (void *)1);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_peek_back()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    FAIL_IF(Nst_llist_peek_back(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    void *value = Nst_llist_peek_back(&llist);
    FAIL_IF(llist.len != 2);
    FAIL_IF(value != (void *)2);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_push_llnode()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = Nst_llnode_new((void *)1, false);
    Nst_LLNode *node2 = Nst_llnode_new((void *)2, false);
    Nst_llist_push_llnode(&llist, node1);
    FAIL_IF(llist.len != 1);
    FAIL_IF(llist.head != node1);
    FAIL_IF(llist.tail != node1);
    Nst_llist_push_llnode(&llist, node2);
    FAIL_IF(llist.len != 2);
    FAIL_IF(llist.head != node2);
    FAIL_IF(llist.tail != node1);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_append_llnode()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = Nst_llnode_new((void *)1, false);
    Nst_LLNode *node2 = Nst_llnode_new((void *)2, false);
    Nst_llist_append_llnode(&llist, node1);
    FAIL_IF(llist.len != 1);
    FAIL_IF(llist.head != node1);
    FAIL_IF(llist.tail != node1);
    Nst_llist_append_llnode(&llist, node2);
    FAIL_IF(llist.len != 2);
    FAIL_IF(llist.head != node1);
    FAIL_IF(llist.tail != node2);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_pop_llnode()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_LLNode *node1 = NULL;
    Nst_LLNode *node2 = NULL;

    FAIL_IF(Nst_llist_pop_llnode(&llist) != NULL);
    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    node1 = Nst_llist_pop_llnode(&llist);
    FAIL_IF(llist.len != 1);
    FAIL_IF(node1->value != (void *)1);
    node2 = Nst_llist_pop_llnode(&llist);
    FAIL_IF(llist.head != NULL);
    FAIL_IF(llist.tail != NULL);
    FAIL_IF(node2->value != (void *)2);

    if (node1 != NULL)
        Nst_free(node1);
    if (node2 != NULL)
        Nst_free(node2);
    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_empty()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList llist;
    Nst_llist_init(&llist);

    Nst_llist_append(&llist, (void *)1, false);
    Nst_llist_append(&llist, (void *)2, false);
    Nst_llist_append(&llist, (void *)3, false);
    Nst_llist_empty(&llist, NULL);
    FAIL_IF(llist.len != 0);
    FAIL_IF(llist.head != NULL);
    FAIL_IF(llist.tail != NULL);

    void *alloc_v1 = Nst_raw_malloc(1);
    void *alloc_v2 = Nst_raw_malloc(1);
    void *alloc_v3 = Nst_raw_malloc(1);
    Nst_llist_append(&llist, alloc_v1, true);
    Nst_llist_append(&llist, alloc_v2, true);
    Nst_llist_append(&llist, alloc_v3, true);
    Nst_llist_empty(&llist, Nst_free);
    FAIL_IF(llist.len != 0);
    FAIL_IF(llist.head != NULL);
    FAIL_IF(llist.tail != NULL);

    Nst_llist_empty(&llist, NULL);
    return result;
}

TestResult test_llist_move_nodes()
{
    TestResult result = TEST_SUCCESS;
    Nst_LList from, to;
    Nst_llist_init(&from);
    Nst_llist_init(&to);

    Nst_llist_append(&from, (void *)1, false);
    Nst_llist_append(&from, (void *)2, false);
    Nst_llist_append(&from, (void *)3, false);
    Nst_llist_move_nodes(&from, &to);

    FAIL_IF(from.len != 0);
    FAIL_IF(from.head != NULL);
    FAIL_IF(from.tail != NULL);
    FAIL_IF(to.len != 3);
    FAIL_IF(to.head->value != (void *)1);
    FAIL_IF(to.tail->value != (void *)3);
    FAIL_IF(to.head->next->value != (void *)2);

    Nst_llist_empty(&from, NULL);
    Nst_llist_empty(&to, NULL);
    return result;
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

TestResult test_sbuffer_pop()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_at()
{
    return TEST_NOT_IMPL;
}

TestResult test_sbuffer_shrink_auto()
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
