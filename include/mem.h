#ifndef MEM_H
#define MEM_H

#include "typedefs.h"
#include "error.h"

#define nst_raw_free nst_free

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Nst_Buffer
{
    usize len;
    usize size;
    i8 *data;
}
Nst_Buffer;

EXPORT void *nst_raw_malloc(usize size);
EXPORT void *nst_raw_calloc(usize count, usize size);
EXPORT void *nst_raw_realloc(void *block, usize size);

EXPORT void *nst_malloc(usize count, usize size, Nst_OpErr *err);
EXPORT void *nst_calloc(usize count,
                        usize size,
                        void *init_value,
                        Nst_OpErr *err);
EXPORT void *nst_realloc(void *prev_block,
                         usize new_count,
                         usize size,
                         usize prev_count,
                         Nst_OpErr *err);
EXPORT void *nst_crealloc(void *prev_block,
                          usize new_count,
                          usize size,
                          usize prev_count,
                          void *init_value,
                          Nst_OpErr *err);
EXPORT void nst_free(void *block);

EXPORT bool nst_buffer_init(Nst_Buffer *buf, usize initial_size, Nst_OpErr *err);
EXPORT bool nst_buffer_expand_by(Nst_Buffer *buf, usize amount, Nst_OpErr *err);
EXPORT bool nst_buffer_expand_to(Nst_Buffer *buf, usize size, Nst_OpErr *err);
EXPORT void nst_buffer_fit(Nst_Buffer *buf);
EXPORT bool nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str, Nst_OpErr *err);
EXPORT bool nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str, Nst_OpErr *err);
EXPORT bool nst_buffer_append_char(Nst_Buffer *buf, i8 ch, Nst_OpErr *err);
EXPORT Nst_StrObj *nst_buffer_to_string(Nst_Buffer *buf, Nst_OpErr *err);
EXPORT void nst_buffer_destroy(Nst_Buffer *buf);

#ifdef __cplusplus
}
#endif

#endif // !MEM_H
