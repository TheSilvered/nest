#ifndef MEM_H
#define MEM_H

#include <typedefs.h>

#define nst_raw_free nst_free

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void *nst_raw_malloc(usize size);
EXPORT void *nst_raw_calloc(usize count, usize size);
EXPORT void *nst_raw_realloc(void *block, usize size);

EXPORT void *nst_malloc(usize count, usize size);
EXPORT void *nst_calloc(usize count, usize size, void *init_value);
EXPORT void *nst_realloc(void *prev_block, usize new_count, usize size);
EXPORT void *nst_crealloc(void *prev_block,
                          usize new_count,
                          usize size,
                          usize prev_count,
                          void *init_value);
EXPORT void nst_free(void *block);

#ifdef __cplusplus
}
#endif

#endif // !MEM_H
