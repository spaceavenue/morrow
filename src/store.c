#include "internal.h"
#include <string.h>
#include <sys/mman.h>

static store_t global_store;
static int global_live;

/* Committed chunk bitmap. By default, 64GiB / 2MiB 64 bits = 512 words. Static
 * because using committing memory shouldn't require allocating memory. This
 * allows us to bootstrap ourself.
 */
#define COMMIT_WORDS (MAX_STORE_SIZE / CHUNK_SIZE / 64u)
static uint64_t global_commit_map[COMMIT_WORDS];

/* Get the commit bit for the given chunk. */
static inline int commit_bit(uint32_t chunk) {
  return (int)((global_commit_map[chunk >> 6] >> (chunk & 63u)) & 1u);
}

/* Mark the given chunk as committed. */
static inline void commit_mark(uint32_t chunk) {
  global_commit_map[chunk >> 6] |= 1u << (chunk & 63u);
}

/* Return a pointer to global store if it is live */
const store_t *store(void) { return global_live ? &global_store : NULL; }

static uint32_t floor_log_u32(uint32_t v) {
  uint32_t r = 0;
  while (v > 1) {
    v = v >> 1;
    r++;
  }
  return r;
}

/* Initialize the store. */
int store_init(size_t bytes) {
  if (global_live)
    return -1;

  if (bytes == 0)
    bytes = STORE_SIZE;
  if (bytes > MAX_STORE_SIZE)
    bytes = MAX_STORE_SIZE;
  if (bytes < CHUNK_SIZE)
    return CHUNK_SIZE;

  bytes = bytes & ~(PAGE_SIZE);

  uint32_t pages = (uint32_t)(bytes >> PAGE_SHIFT);
  uint32_t max_order = floor_log_u32(pages);
  pages = 1u << max_order;
  size_t size = (size_t)pages << PAGE_SHIFT;

  size_t reserve = size + STORE_BASE_ALIGN;
  void *raw = mmap(NULL, reserve, PROT_NONE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
  if (raw == MAP_FAILED) {
    abort();
  }

  uintptr_t start = (uintptr_t)raw;
  uintptr_t aligned =
      (start + (STORE_BASE_ALIGN - 1)) & ~(uintptr_t)(STORE_BASE_ALIGN - 1);

  if (aligned > start)
    (void)munmap(raw, (aligned - start));

  uintptr_t end = start + reserve;
  if (end > aligned + size)
    (void)munmap((void *)(aligned + size), (size_t)(end - aligned - size));

#ifdef MADV_HUGEPAGE
  (void)madvise((void *)aligned, size, MADV_HUGEPAGE);
#endif

  memset(global_commit_map, 0, sizeof(global_commit_map));

  global_store.base = (char *)aligned;
  global_store.size = size;
  global_store.pages = pages;
  global_store.max_order = max_order;
  global_store.committed_chunks = 0;
  global_store.commit_syscalls = 0;
  global_live = 1;

  return 0;
}

/* Destroy the store. */
void store_destroy(void) {
  if (!global_live)
    return;
  (void)munmap(global_store.base, global_store.size);
  memset(&global_store, 0, sizeof(global_store));
  memset(global_commit_map, 0, sizeof(global_commit_map));
  global_live = 0;
}

/* commit `npages` to the global store. */
int store_commit(uint32_t page, uint32_t npages) {
  if (!global_live || npages == 0)
    return global_live ? 0 : -1;

  if (page >= global_store.pages || npages > global_store.pages - page)
    return -1;

  uint32_t first = page / CHUNK_PAGES;
  uint32_t last = (page + npages - 1) / CHUNK_PAGES;

  for (uint32_t chunk = first; chunk <= last; ++chunk) {
    if (commit_bit(chunk))
      continue;
    void *addr = global_store.base + ((size_t)chunk << CHUNK_SHIFT);
    if (mprotect(addr, CHUNK_SIZE, PROT_READ | PROT_WRITE) != 0)
      return -1;
    commit_mark(chunk);
    global_store.committed_chunks++;
    global_store.commit_syscalls++;
  }
  return 0;
}
