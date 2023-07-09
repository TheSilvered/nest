#ifndef MEM_H
#define MEM_H

#include "typedefs.h"
#include "error.h"

#define Nst_raw_free Nst_free

#define Nst_malloc_c(count, type) \
    ((type *)Nst_malloc(count, sizeof(type)))

#define Nst_calloc_c(count, type, init_value) \
    ((type *)Nst_calloc(count, sizeof(type), (void *)(init_value)))

#define Nst_realloc_c(prev_block, new_count, type, prev_count) \
    ((type *)Nst_realloc( \
        (void *)(prev_block), \
        new_count, \
        sizeof(type), \
        prev_count))

#define Nst_crealloc_c(prev_block, new_count, type, prev_count, init_value) \
    ((type *)Nst_crealloc( \
        (void *)(prev_block), \
        new_count, \
        sizeof(type), \
        prev_count, \
        (void *)(init_value)))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

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

NstEXP void *NstC Nst_raw_malloc(usize size);
NstEXP void *NstC Nst_raw_calloc(usize count, usize size);
NstEXP void *NstC Nst_raw_realloc(void *block, usize size);

NstEXP void *NstC Nst_malloc(usize count, usize size);
NstEXP void *NstC Nst_calloc(usize count, usize size, void *init_value);
NstEXP void *NstC Nst_realloc(void *prev_block, usize new_count, usize size,
                              usize prev_count);
NstEXP void *NstC Nst_crealloc(void *prev_block, usize new_count, usize size,
                               usize prev_count, void *init_value);
NstEXP void NstC Nst_free(void *block);

NstEXP bool NstC Nst_sbuffer_init(Nst_SizedBuffer *buf, usize unit_size,
                                  usize count);
NstEXP bool NstC Nst_sbuffer_expand_by(Nst_SizedBuffer *buf, usize amount);
NstEXP bool NstC Nst_sbuffer_expand_to(Nst_SizedBuffer *buf, usize count);
NstEXP void NstC Nst_sbuffer_fit(Nst_SizedBuffer *buf);
NstEXP bool NstC Nst_sbuffer_append(Nst_SizedBuffer *buf, void *element);
NstEXP void NstC Nst_sbuffer_destroy(Nst_SizedBuffer *buf);

NstEXP bool NstC Nst_buffer_init(Nst_Buffer *buf, usize initial_size);
NstEXP bool NstC Nst_buffer_expand_by(Nst_Buffer *buf, usize amount);
NstEXP bool NstC Nst_buffer_expand_to(Nst_Buffer *buf, usize size);
NstEXP void NstC Nst_buffer_fit(Nst_Buffer *buf);
NstEXP bool NstC Nst_buffer_append(Nst_Buffer *buf, Nst_StrObj *str);
NstEXP bool NstC Nst_buffer_append_c_str(Nst_Buffer *buf, const i8 *str);
NstEXP bool NstC Nst_buffer_append_char(Nst_Buffer *buf, i8 ch);
NstEXP Nst_StrObj *NstC Nst_buffer_to_string(Nst_Buffer *buf);
NstEXP void NstC Nst_buffer_destroy(Nst_Buffer *buf);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !MEM_H
