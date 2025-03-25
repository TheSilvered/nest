#include "tests.h"

int main() {
    Nst_init(NULL);

#ifdef Nst_MSVC
    _Nst_console_mode_init();
#endif

    Nst_printf("Tests running on Nest version %s\n\n", Nst_VERSION);

    test_init();

    // argv_parser.h

    RUN_TEST(test_cl_args_parse);

#ifdef Nst_MSVC
    RUN_TEST(test_wargv_to_argv);
#endif

    // dyn_array.h

    RUN_TEST(test_da_init);
    RUN_TEST(test_da_init_copy);
    RUN_TEST(test_da_reserve);
    RUN_TEST(test_da_append);
    RUN_TEST(test_da_remove_swap);
    RUN_TEST(test_da_remove_shift);
    RUN_TEST(test_da_at);
    RUN_TEST(test_da_clear);

    // encoding.h

    RUN_TEST(test_check_bytes);
    RUN_TEST(test_to_utf32);
    RUN_TEST(test_from_utf32);
    RUN_TEST(test_utf16_to_utf8);
    RUN_TEST(test_encoding_translate);
    RUN_TEST(test_encoding_check);
    RUN_TEST(test_encoding_char_len);
    RUN_TEST(test_encoding_utf8_char_len);
    RUN_TEST(test_char_to_wchar_t);
    RUN_TEST(test_wchar_t_to_char);
    RUN_TEST(test_is_valid_cp);
    RUN_TEST(test_is_non_character);
    RUN_TEST(test_check_bom);
    RUN_TEST(test_encoding_detect);
    RUN_TEST(test_encoding_from_name);
    RUN_TEST(test_encoding_to_single_byte);

    // error.h

    RUN_TEST(test_tb_add_pos);
    RUN_TEST(test_error_set);
    RUN_TEST(test_error_setc);
    RUN_TEST(test_error_setf);
    RUN_TEST(test_error_add_pos);
    RUN_TEST(test_error_occurred);
    RUN_TEST(test_error_get);
    RUN_TEST(test_error_clear);

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
    RUN_TEST(test_fmt_objs);
    RUN_TEST(test_repr);

    // function.h

    RUN_TEST(test_func_set_vt);

    // hash.h

    RUN_TEST(test_obj_hash);

    // iter.h

    RUN_TEST(test_iter_start);
    RUN_TEST(test_iter_get_val);

    // lib_import.h

    RUN_TEST(test_extract_args);

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

    RUN_TEST(test_map_new);
    RUN_TEST(test_map_copy);
    RUN_TEST(test_map_set);
    RUN_TEST(test_map_get);
    RUN_TEST(test_map_drop);
    RUN_TEST(test_map_set_str);
    RUN_TEST(test_map_get_str);
    RUN_TEST(test_map_drop_str);
    RUN_TEST(test_map_next);
    RUN_TEST(test_map_prev);

    // mem.h

    RUN_TEST(test_malloc);
    RUN_TEST(test_calloc);
    RUN_TEST(test_realloc);
    RUN_TEST(test_crealloc);
    RUN_TEST(test_memset);

    // sequence.h

    RUN_TEST(test_seq_new);
    RUN_TEST(test_seq_create);
    RUN_TEST(test_seq_create_c);
    RUN_TEST(test_seq_set);
    RUN_TEST(test_seq_get);
    RUN_TEST(test_seq_copy);
    RUN_TEST(test_vector_append);
    RUN_TEST(test_vector_remove);
    RUN_TEST(test_vector_pop);

    // simple_types.h

    RUN_TEST(test_number_to_u8);
    RUN_TEST(test_number_to_int);
    RUN_TEST(test_number_to_i32);
    RUN_TEST(test_number_to_i64);
    RUN_TEST(test_number_to_f32);
    RUN_TEST(test_number_to_f64);
    RUN_TEST(test_obj_to_bool);

    // str.h

    RUN_TEST(test_str_new_c_raw);
    RUN_TEST(test_str_new_c);
    RUN_TEST(test_str_new);
    RUN_TEST(test_str_new_allocated);
    RUN_TEST(test_str_new_len);
    RUN_TEST(test_str_from_sv);
    RUN_TEST(test_str_from_sb);
    RUN_TEST(test_str_copy);
    RUN_TEST(test_str_repr);
    RUN_TEST(test_str_get_obj);
    RUN_TEST(test_str_get);
    RUN_TEST(test_str_next);
    RUN_TEST(test_str_next_obj);
    RUN_TEST(test_str_next_utf32);
    RUN_TEST(test_str_next_utf8);
    RUN_TEST(test_str_parse_int);
    RUN_TEST(test_str_parse_byte);
    RUN_TEST(test_str_parse_real);
    RUN_TEST(test_str_compare);

    // str_builder.h

    RUN_TEST(test_sb_init);
    RUN_TEST(test_sb_reserve);
    RUN_TEST(test_sb_push);
    RUN_TEST(test_sb_push_sv);
    RUN_TEST(test_sb_push_str);
    RUN_TEST(test_sb_push_c);
    RUN_TEST(test_sb_push_cps);
    RUN_TEST(test_sb_push_char);

    // str_view.h

    RUN_TEST(test_sv_new);
    RUN_TEST(test_sv_new_c);
    RUN_TEST(test_sv_from_str);
    RUN_TEST(test_sv_from_sb);
    RUN_TEST(test_sv_next);
    RUN_TEST(test_sv_prev);
    RUN_TEST(test_sv_parse_int);
    RUN_TEST(test_sv_parse_byte);
    RUN_TEST(test_sv_parse_real);
    RUN_TEST(test_sv_compare);
    RUN_TEST(test_sv_lfind);
    RUN_TEST(test_sv_rfind);
    RUN_TEST(test_sv_ltok);
    RUN_TEST(test_sv_rtok);

    Nst_printf("\nTests failed: %" PRIi32 "\n", tests_failed_count());

    Nst_quit();

    Nst_log_alloc_count();
    Nst_log_alloc_info();

    if (tests_failed_count() != 0)
        return 1;
    return 0;
}
