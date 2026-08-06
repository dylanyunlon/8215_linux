#ifndef ADDR2FUNC_H
#define ADDR2FUNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* Flags */
#define FT_FLAG_INLINE  (1u << 0)

/* Bare-metal friendly API: uses linker-provided base address of table.bin */

/* Initialize internal view from memory at linker-provided base. */
int  func_table_init(void);
void func_table_deinit(void);

/* Map address to preferred function name (non-inline preferred). */
const char* addr2func(uint32_t addr);

/* Get full inline chain at address (returns count written to arrays). */
size_t addr2func_chain(uint32_t addr, const char** names, uint8_t* flags, size_t max);

unsigned int ft_get_total_size(void);

#ifdef __cplusplus
}
#endif

#endif /* ADDR2FUNC_H */
