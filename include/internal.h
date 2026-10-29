#ifndef MORROW_INTERNAL_H
#define MORROW_INTERNAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

typedef struct {
  char *base;
  size_t size;
  uint32_t pages;
  uint32_t max_order;
  uint32_t committed_chunks;
  uint64_t commit_syscalls;
} store_t;

int store_init(size_t bytes);
void store_destroy(void);
const store_t *store(void);

int store_commit(uint32_t page, uint32_t npages);

static inline char *store_ptr(uint32_t page) {
  return store()->base + ((size_t)page << PAGE_SHIFT);
}

#endif /* MORROW_INTERNAL_H */
