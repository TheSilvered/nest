#include "tests.h"

int main() {
    Nst_init();

#ifdef Nst_MSVC
    _Nst_console_mode_init();
#endif

    Nst_printf("Tests running on Nest version %s\n\n", Nst_VERSION);

    test_init();

    // argv_parser.h

    test_run(test_cl_args_parse);

#ifdef Nst_MSVC
    test_run(test_wargv_to_argv);
#endif

    // dyn_array.h

    test_run(test_da_init);
    test_run(test_da_init_copy);
    test_run(test_da_reserve);
    test_run(test_da_append);
    test_run(test_da_remove_swap);
    test_run(test_da_remove_shift);
    test_run(test_da_get);
    test_run(test_da_set);
    test_run(test_da_clear);
    test_run(test_pa_init);
    test_run(test_pa_init_copy);
    test_run(test_pa_reserve);
    test_run(test_pa_append);
    test_run(test_pa_remove_swap);
    test_run(test_pa_remove_shift);
    test_run(test_pa_get);
    test_run(test_pa_set);
    test_run(test_pa_clear);

    // encoding.h

    test_run(test_check_bytes);
    test_run(test_to_utf32);
    test_run(test_from_utf32);
    test_run(test_utf16_to_utf8);
    test_run(test_encoding_translate);
    test_run(test_encoding_check);
    test_run(test_encoding_char_len);
    test_run(test_encoding_utf8_char_len);
    test_run(test_char_to_wchar_t);
    test_run(test_wchar_t_to_char);
    test_run(test_cp_is_valid);
    test_run(test_cp_is_non_character);
    test_run(test_check_bom);
    test_run(test_encoding_detect);
    test_run(test_encoding_from_name);
    test_run(test_encoding_to_single_byte);

    // error.h

    test_run(test_span_new);
    test_run(test_span_from_pos);
    test_run(test_span_join);
    test_run(test_span_extend);
    test_run(test_span_start);
    test_run(test_span_end);
    test_run(test_error_set);
    test_run(test_error_setc);
    test_run(test_error_setf);
    test_run(test_error_failed_alloc);
    test_run(test_error_add_span);
    test_run(test_error_occurred);
    test_run(test_error_get);
    test_run(test_error_clear);

    // file.h

    test_run(test_FILE_read);
    test_run(test_FILE_write);
    test_run(test_FILE_flush);
    test_run(test_FILE_tell);
    test_run(test_FILE_seek);
    test_run(test_FILE_close);
    test_run(test_io_result_details);
    test_run(test_fopen_unicode);

    // format.h

    test_run(test_sprintf);
    test_run(test_fmt);
    test_run(test_fmt_objs);
    test_run(test_repr);

    // function.h

    test_run(test_func_set_vt);

    // hash.h

    test_run(test_obj_hash);

    // iter.h

    test_run(test_iter_start_func);
    test_run(test_iter_next_func);
    test_run(test_iter_value);
    test_run(test_iter_start);
    test_run(test_iter_get_val);
    test_run(test_iter_range_new);
    test_run(test_iter_seq_new);
    test_run(test_iter_str_new);
    test_run(test_iter_map_new);

    // lib_import.h

    test_run(test_extract_args);
    test_run(test_obj_custom);
    test_run(test_obj_custom_ex);
    test_run(test_obj_custom_data);

    // llist.h

    test_run(test_llist_push);
    test_run(test_llist_append);
    test_run(test_llist_insert);
    test_run(test_llist_pop);
    test_run(test_llist_peek_front);
    test_run(test_llist_peek_back);
    test_run(test_llist_push_llnode);
    test_run(test_llist_append_llnode);
    test_run(test_llist_pop_llnode);
    test_run(test_llist_empty);
    test_run(test_llist_move_nodes);

    // map.h

    test_run(test_map_new);
    test_run(test_map_copy);
    test_run(test_map_len);
    test_run(test_map_cap);
    test_run(test_map_set);
    test_run(test_map_get);
    test_run(test_map_drop);
    test_run(test_map_set_str);
    test_run(test_map_get_str);
    test_run(test_map_drop_str);
    test_run(test_map_next);
    test_run(test_map_prev);

    // mem.h

    test_run(test_malloc);
    test_run(test_calloc);
    test_run(test_realloc);
    test_run(test_crealloc);
    test_run(test_memset);

    // sequence.h

    test_run(test_seq_new);
    test_run(test_seq_from_objs);
    test_run(test_seq_create);
    test_run(test_seq_create_c);
    test_run(test_seq_len);
    test_run(test_vector_cap);
    test_run(test_seq_objs);
    test_run(test_seq_set);
    test_run(test_seq_get);
    test_run(test_seq_copy);
    test_run(test_vector_append);
    test_run(test_vector_remove);
    test_run(test_vector_pop);

    // simple_types.h

    test_run(test_number_to_u8);
    test_run(test_number_to_int);
    test_run(test_number_to_i32);
    test_run(test_number_to_i64);
    test_run(test_number_to_f32);
    test_run(test_number_to_f64);
    test_run(test_obj_to_bool);

    // source_loader.h

    test_run(test_source_from_sv);

    // str.h

    test_run(test_str_new_c);
    test_run(test_str_new);
    test_run(test_str_new_allocated);
    test_run(test_str_new_len);
    test_run(test_str_from_sv);
    test_run(test_str_from_sb);
    test_run(test_str_copy);
    test_run(test_str_repr);
    test_run(test_str_get_obj);
    test_run(test_str_get);
    test_run(test_str_next);
    test_run(test_str_next_obj);
    test_run(test_str_next_utf32);
    test_run(test_str_next_utf8);
    test_run(test_str_parse_int);
    test_run(test_str_parse_byte);
    test_run(test_str_parse_real);
    test_run(test_str_compare);

    // str_builder.h

    test_run(test_sb_init);
    test_run(test_sb_reserve);
    test_run(test_sb_push);
    test_run(test_sb_push_sv);
    test_run(test_sb_push_str);
    test_run(test_sb_push_c);
    test_run(test_sb_push_cps);
    test_run(test_sb_push_char);

    // str_view.h

    test_run(test_sv_new);
    test_run(test_sv_new_c);
    test_run(test_sv_from_str);
    test_run(test_sv_from_sb);
    test_run(test_sv_next);
    test_run(test_sv_prev);
    test_run(test_sv_parse_int);
    test_run(test_sv_parse_byte);
    test_run(test_sv_parse_real);
    test_run(test_sv_compare);
    test_run(test_sv_lfind);
    test_run(test_sv_rfind);
    test_run(test_sv_ltok);
    test_run(test_sv_rtok);

    Nst_printf("\nTests failed: %" PRIi32 "\n", tests_failed_count());

    Nst_quit();

    Nst_log_alloc_count();
    Nst_log_alloc_info();

    if (tests_failed_count() != 0)
        return 1;
    return 0;
}
