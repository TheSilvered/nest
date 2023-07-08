#ifndef MEM_H
#define MEM_H

#include "typedefs.h"
#include "error.h"

#define nst_raw_free nst_free

#define nst_malloc_c(count, type) \
    ((type *)nst_malloc(count, sizeof(type)))

#define nst_calloc_c(count, type, init_value) \
    ((type *)nst_calloc(count, sizeof(type), (void *)(init_value)))

#define nst_realloc_c(prev_block, new_count, type, prev_count) \
    ((type *)nst_realloc( \
        (void *)(prev_block), \
        new_count, \
        sizeof(type), \
        prev_count))

#define nst_crealloc_c(prev_block, new_count, type, prev_count, init_value) \
    ((type *)nst_crealloc( \
        (void *)(prev_block), \
        new_count, \
        sizeof(type), \
        prev_count, \
        (void *)(init_value)))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Nst_SizedBuffer
{
    usize len;
    usize size;
    usize unit_size;
    void *data;
}
Nst_SizedBuffer;

typedef struct _Nst_Buffer
{
    usize len;
    usize size;
    usize unit_size;
    i8 *data;
}
Nst_Buffer;

NstEXP void *nst_raw_malloc(usize size);
NstEXP void *nst_raw_calloc(usize count, usize size);
NstEXP void *nst_raw_realloc(void *block, usize size);

NstEXP void *nst_malloc(usize count, usize size);
NstEXP void *nst_calloc(usize count,
                        usize size,
                        void *init_value);
NstEXP void *nst_realloc(void *prev_block,
                         usize new_count,
                         usize size,
                         usize prev_count);
NstEXP void *nst_crealloc(void *prev_block,
                          usize new_count,
                          usize size,
                          usize prev_count,
                          void *init_value);
NstEXP void nst_free(void *block);

NstEXP bool nst_sbuffer_init(Nst_SizedBuffer *buf, usize unit_size, usize count);
NstEXP bool nst_sbuffer_expand_by(Nst_SizedBuffer *buf, usize amount);
NstEXP bool nst_sbuffer_expand_to(Nst_SizedBuffer *buf, usize count);
NstEXP void nst_sbuffer_fit(Nst_SizedBuffer *buf);
NstEXP bool nst_sbuffer_append(Nst_SizedBuffer *buf, void *element);
NstEXP void nst_sbuffer_destroy(Nst_SizedBuffer *buf);

NstEXP bool nst_buffer_init(Nst_Buffer *buf, usize initial_size);
NstEXP bool nst_buffer_expand_by(Nst_Buffer *buf, usize amount);
NstEXP bool nst_buffer_expand_to(Nst_Buffer *buf, usize size);
NstEXP void nst_buffer_fit(Nst_Buffer *buf);
NstEXP bool nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str);
NstEXP bool nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str);
NstEXP bool nst_buffer_append_char(Nst_Buffer *buf, i8 ch);
NstEXP Nst_StrObj *nst_buffer_to_string(Nst_Buffer *buf);
NstEXP void nst_buffer_destroy(Nst_Buffer *buf);

#ifdef __cplusplus
}
#endif

#endif // !MEM_H
