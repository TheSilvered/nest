#include "tests.h"

int main() {
    Nst_init();

#ifdef Nst_MSVC
    _Nst_console_mode_init();
#endif

    Nst_printf("Tests running on Nest version %s\n\n", Nst_VERSION);

    test_init();

    // argv_parser.h

    run_test(test_cl_args_parse);

#ifdef Nst_MSVC
    run_test(test_wargv_to_argv);
#endif

    // dyn_array.h

    run_test(test_da_init);
    run_test(test_da_init_copy);
    run_test(test_da_reserve);
    run_test(test_da_append);
    run_test(test_da_remove_swap);
    run_test(test_da_remove_shift);
    run_test(test_da_get);
    run_test(test_da_set);
    run_test(test_da_clear);
    run_test(test_pa_init);
    run_test(test_pa_init_copy);
    run_test(test_pa_reserve);
    run_test(test_pa_append);
    run_test(test_pa_remove_swap);
    run_test(test_pa_remove_shift);
    run_test(test_pa_get);
    run_test(test_pa_set);
    run_test(test_pa_clear);

    // encoding.h

    run_test(test_check_bytes);
    run_test(test_to_utf32);
    run_test(test_from_utf32);
    run_test(test_utf16_to_utf8);
    run_test(test_encoding_translate);
    run_test(test_encoding_check);
    run_test(test_encoding_char_len);
    run_test(test_encoding_utf8_char_len);
    run_test(test_char_to_wchar_t);
    run_test(test_wchar_t_to_char);
    run_test(test_cp_is_valid);
    run_test(test_cp_is_non_character);
    run_test(test_check_bom);
    run_test(test_encoding_detect);
    run_test(test_encoding_from_name);
    run_test(test_encoding_to_single_byte);

    // error.h

    run_test(test_span_new);
    run_test(test_span_from_pos);
    run_test(test_span_join);
    run_test(test_span_extend);
    run_test(test_span_start);
    run_test(test_span_end);
    run_test(test_error_set);
    run_test(test_error_setc);
    run_test(test_error_setf);
    run_test(test_error_failed_alloc);
    run_test(test_error_add_span);
    run_test(test_error_occurred);
    run_test(test_error_get);
    run_test(test_error_clear);

    // file.h

    run_test(test_FILE_read);
    run_test(test_FILE_write);
    run_test(test_FILE_flush);
    run_test(test_FILE_tell);
    run_test(test_FILE_seek);
    run_test(test_FILE_close);
    run_test(test_io_result_details);
    run_test(test_fopen_unicode);

    // format.h

    run_test(test_sprintf);
    run_test(test_fmt);
    run_test(test_fmt_objs);
    run_test(test_repr);

    // function.h

    run_test(test_func_set_vt);

    // hash.h

    run_test(test_obj_hash);

    // iter.h

    run_test(test_iter_start);
    run_test(test_iter_get_val);

    // lib_import.h

    run_test(test_extract_args);

    // llist.h

    run_test(test_llist_push);
    run_test(test_llist_append);
    run_test(test_llist_insert);
    run_test(test_llist_pop);
    run_test(test_llist_peek_front);
    run_test(test_llist_peek_back);
    run_test(test_llist_push_llnode);
    run_test(test_llist_append_llnode);
    run_test(test_llist_pop_llnode);
    run_test(test_llist_empty);
    run_test(test_llist_move_nodes);

    // map.h

    run_test(test_map_new);
    run_test(test_map_copy);
    run_test(test_map_set);
    run_test(test_map_get);
    run_test(test_map_drop);
    run_test(test_map_set_str);
    run_test(test_map_get_str);
    run_test(test_map_drop_str);
    run_test(test_map_next);
    run_test(test_map_prev);

    // mem.h

    run_test(test_malloc);
    run_test(test_calloc);
    run_test(test_realloc);
    run_test(test_crealloc);
    run_test(test_memset);

    // sequence.h

    run_test(test_seq_new);
    run_test(test_seq_create);
    run_test(test_seq_create_c);
    run_test(test_seq_set);
    run_test(test_seq_get);
    run_test(test_seq_copy);
    run_test(test_vector_append);
    run_test(test_vector_remove);
    run_test(test_vector_pop);

    // simple_types.h

    run_test(test_number_to_u8);
    run_test(test_number_to_int);
    run_test(test_number_to_i32);
    run_test(test_number_to_i64);
    run_test(test_number_to_f32);
    run_test(test_number_to_f64);
    run_test(test_obj_to_bool);

    // str.h

    run_test(test_str_new_c_raw);
    run_test(test_str_new_c);
    run_test(test_str_new);
    run_test(test_str_new_allocated);
    run_test(test_str_new_len);
    run_test(test_str_from_sv);
    run_test(test_str_from_sb);
    run_test(test_str_copy);
    run_test(test_str_repr);
    run_test(test_str_get_obj);
    run_test(test_str_get);
    run_test(test_str_next);
    run_test(test_str_next_obj);
    run_test(test_str_next_utf32);
    run_test(test_str_next_utf8);
    run_test(test_str_parse_int);
    run_test(test_str_parse_byte);
    run_test(test_str_parse_real);
    run_test(test_str_compare);

    // str_builder.h

    run_test(test_sb_init);
    run_test(test_sb_reserve);
    run_test(test_sb_push);
    run_test(test_sb_push_sv);
    run_test(test_sb_push_str);
    run_test(test_sb_push_c);
    run_test(test_sb_push_cps);
    run_test(test_sb_push_char);

    // str_view.h

    run_test(test_sv_new);
    run_test(test_sv_new_c);
    run_test(test_sv_from_str);
    run_test(test_sv_from_sb);
    run_test(test_sv_next);
    run_test(test_sv_prev);
    run_test(test_sv_parse_int);
    run_test(test_sv_parse_byte);
    run_test(test_sv_parse_real);
    run_test(test_sv_compare);
    run_test(test_sv_lfind);
    run_test(test_sv_rfind);
    run_test(test_sv_ltok);
    run_test(test_sv_rtok);

    Nst_printf("\nTests failed: %" PRIi32 "\n", tests_failed_count());

    Nst_quit();

    Nst_log_alloc_count();
    Nst_log_alloc_info();

    if (tests_failed_count() != 0)
        return 1;
    return 0;
}
