#include "nest.h"
#include "test_nest.h"

int main() {
    Nst_init(NULL);

#ifdef Nst_WIN
    _Nst_set_console_mode();
#endif

    Nst_printf("Tests running on Nest version %s\n\n", Nst_VERSION);

    test_init();

    // argv_parser.h

    RUN_TEST(test_parse_args);

#ifdef Nst_WIN
    RUN_TEST(test_wargv_to_argv);
#endif

    // encoding.h

    RUN_TEST(test_check_bytes);
    RUN_TEST(test_to_utf32);
    RUN_TEST(test_from_utf32);
    RUN_TEST(test_utf16_to_utf8);
    RUN_TEST(test_translate_cp);
    RUN_TEST(test_check_string_cp);
    RUN_TEST(test_string_char_len);
    RUN_TEST(test_string_utf8_char_len);
    RUN_TEST(test_char_to_wchar_t);
    RUN_TEST(test_wchar_t_to_char);
    RUN_TEST(test_is_valid_cp);
    RUN_TEST(test_is_non_character);
    RUN_TEST(test_check_bom);
    RUN_TEST(test_detect_encoding);
    RUN_TEST(test_encoding_from_name);
    RUN_TEST(test_single_byte_cp);

    // error.h

    RUN_TEST(test_set_error);
    RUN_TEST(test_set_error_c);
    RUN_TEST(test_error_occurred);
    RUN_TEST(test_error_get);
    RUN_TEST(test_error_clear);
    RUN_TEST(test_set_error_internal);

    // file.h

    RUN_TEST(test_FILE_read);
    RUN_TEST(test_FILE_write);
    RUN_TEST(test_FILE_flush);
    RUN_TEST(test_FILE_tell);
    RUN_TEST(test_FILE_seek);
    RUN_TEST(test_FILE_close);
    RUN_TEST(test_io_result_details);
    RUN_TEST(test_fopen_unicode);

    // format.h

    RUN_TEST(test_sprintf);
    RUN_TEST(test_fmt);

    // function.h

    RUN_TEST(test_func_set_vt);

    // hash.h

    RUN_TEST(test_obj_hash);

    // iter.h

    RUN_TEST(test_iter_start);
    RUN_TEST(test_iter_get_val);

    // lib_import.h

    RUN_TEST(test_extract_arg_values);

    // llist.h

    RUN_TEST(test_llist_push);
    RUN_TEST(test_llist_append);
    RUN_TEST(test_llist_insert);
    RUN_TEST(test_llist_pop);
    RUN_TEST(test_llist_peek_front);
    RUN_TEST(test_llist_peek_back);
    RUN_TEST(test_llist_push_llnode);
    RUN_TEST(test_llist_append_llnode);
    RUN_TEST(test_llist_pop_llnode);
    RUN_TEST(test_llist_empty);
    RUN_TEST(test_llist_move_nodes);

    // map.h

    RUN_TEST(test_map_set);
    RUN_TEST(test_map_get);
    RUN_TEST(test_map_drop);
    RUN_TEST(test_map_copy);
    RUN_TEST(test_map_get_next_idx);
    RUN_TEST(test_map_get_prev_idx);
    RUN_TEST(test_map_set_str);
    RUN_TEST(test_map_get_str);
    RUN_TEST(test_map_drop_str);

    // mem.h

    RUN_TEST(test_alloc);
    RUN_TEST(test_sbuffer_expand_by);
    RUN_TEST(test_sbuffer_expand_to);
    RUN_TEST(test_sbuffer_fit);
    RUN_TEST(test_sbuffer_append);
    RUN_TEST(test_sbuffer_pop);
    RUN_TEST(test_sbuffer_at);
    RUN_TEST(test_sbuffer_shrink_auto);
    RUN_TEST(test_sbuffer_copy);
    RUN_TEST(test_buffer_expand_by);
    RUN_TEST(test_buffer_expand_to);
    RUN_TEST(test_buffer_fit);
    RUN_TEST(test_buffer_append);
    RUN_TEST(test_buffer_append_c_str);
    RUN_TEST(test_buffer_append_str);
    RUN_TEST(test_buffer_append_char);
    RUN_TEST(test_buffer_to_string);
    RUN_TEST(test_buffer_copy);

    // sequence.h

    RUN_TEST(test_seq_set);
    RUN_TEST(test_seq_get);
    RUN_TEST(test_seq_copy);
    RUN_TEST(test_vector_append);
    RUN_TEST(test_vector_remove);
    RUN_TEST(test_vector_pop);
    RUN_TEST(test_array_vector_create);
    RUN_TEST(test_array_vector_create_c);

    // simple_types.h

    RUN_TEST(test_number_to_u8);
    RUN_TEST(test_number_to_int);
    RUN_TEST(test_number_to_i32);
    RUN_TEST(test_number_to_i64);
    RUN_TEST(test_number_to_f32);
    RUN_TEST(test_number_to_f64);
    RUN_TEST(test_obj_to_bool);

    // str.h

    RUN_TEST(test_string_copy);
    RUN_TEST(test_string_repr);
    RUN_TEST(test_string_get);
    RUN_TEST(test_string_next_ch);
    RUN_TEST(test_string_new_c_raw);
    RUN_TEST(test_string_new_c);
    RUN_TEST(test_string_new);
    RUN_TEST(test_string_parse_int);
    RUN_TEST(test_string_parse_byte);
    RUN_TEST(test_string_parse_real);
    RUN_TEST(test_string_compare);
    RUN_TEST(test_string_find);
    RUN_TEST(test_string_rfind);

    Nst_quit();

#ifdef Nst_COUNT_ALLOC
    Nst_log_alloc_count();
#endif

    return 0;
}
