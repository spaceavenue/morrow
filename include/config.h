#ifndef MORROW_CONFIG_H
#define MORROW_CONFIG_H

#include <stddef.h>
#include <stdint.h>

/* Power-of-two for 4KiB value */
#define PAGE_SHIFT 12

/* Smallest chunk managed by the page heap (4KiB default) */
#define PAGE_SIZE ((size_t)1 << PAGE_SHIFT)

/* Size of the backing store (32 GiB default) */
#ifndef STORE_SIZE
#define STORE_SIZE ((size_t)32 << 30)
#endif

/* Maximum size of the backing store (64 GiB default) */
#define MAX_STORE_SIZE ((size_t)64 << 30)

/* Power-of-two for 2MiB value */
#define CHUNK_SHIFT 21

/* Size of smallest chunk committed to OS */
#define CHUNK_SIZE ((size_t)1 << CHUNK_SHIFT)

/* Number of `PAGE_SIZE` pages to fit `CHUNK_SIZE` */
#define CHUNK_PAGES (CHUNK_SIZE / PAGE_SIZE)

/* Alignment that the store is guranteed to have. */
#define STORE_BASE_ALIGN CHUNK_SIZE

/* Smallest allocation and guaranteed alignment */
#define MIN_ALIGN 16

#endif /* MORROW_CONFIG_H */
