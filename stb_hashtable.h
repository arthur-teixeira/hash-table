#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef size_t (*hasher_t)(void *, const char *);

typedef struct hash_position_t {
  bool in_use;
  const char *key;
  void *value;
} hash_position_t;

typedef enum {
  PROBE_LINEAR,
  PROBE_QUADRATIC, // TODO: implement
  PROBE_DOUBLE_HASH,
} probe_strategy;

typedef struct hash_options_t {
  hasher_t hasher;
  hasher_t double_hasher;
  probe_strategy strategy;
  size_t size;
} hash_options_t;

typedef struct hash_table_t {
  probe_strategy strategy;
  hasher_t hasher;
  hasher_t double_hasher;
  size_t size;
  size_t p;
  hash_position_t *values;
} hash_table_t;

void hash_table_delete(hash_table_t *table, const char *key);
void *hash_table_lookup(hash_table_t *table, const char *key);
void hash_table_insert(hash_table_t *table, char *key, void *value);
void hash_table_init(hash_table_t *table);
void hash_table_init_ex(hash_table_t *table, hash_options_t options);

#endif // HASH_TABLE_H

// #define HASH_TABLE_IMPLEMENTATION // REMOVE
#ifdef HASH_TABLE_IMPLEMENTATION

#ifndef HASH_TABLE_MALLOC
#define HASH_TABLE_MALLOC(nmemb) malloc(nmemb)
#endif

#ifndef HASH_TABLE_FREE
#define HASH_TABLE_FREE(ptr) free(ptr)
#endif

int string_as_int(const char *key) {
  int acc = 1;
  for (size_t i = 0; i < strlen(key); i++) {
    acc *= key[i] + i;
  }

  return acc;
}

// "Introduction to Algorithms, third edition", Cormen et al., 13.3.2 p:263
// "The Art of Computer Programming, Volume 3, Sorting and Searching", D.E.
// Knuth, 6.4 p:516
size_t knuth_hash(void *t, const char *key) {
  hash_table_t *table = t;
  size_t knuth = 2654435769L;
  size_t key_as_int = string_as_int(key);
  size_t hash = (key_as_int * knuth) >> (32 - table->p);

  return hash % table->size;
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
size_t fnv_hash(void *t, const char *key) {
  hash_table_t *table = t;
  size_t fnv_prime = 1099511628211U;
  size_t fnv_offset = 14695981039346656037U;

  size_t hash = fnv_offset;

  for (size_t i = 0; i < strlen(key); i++, key++) {
    hash *= fnv_prime;
    hash ^= (*key);
  }

  return hash % table->size;
}

void hash_table_init_ex(hash_table_t *table, hash_options_t options) {
  if (options.hasher) {
    table->hasher = options.hasher;
  } else {
    table->hasher = knuth_hash;
  }

  if (options.strategy == PROBE_DOUBLE_HASH) {
    if (options.double_hasher) {
      table->double_hasher = options.double_hasher;
    } else {
      table->double_hasher = fnv_hash;
    }
  }
  table->strategy = options.strategy;
  table->p = rand() % 32;
  table->size = options.size;
  table->values = HASH_TABLE_MALLOC(options.size * sizeof(hash_position_t));
  memset(table->values, 0, options.size * sizeof(hash_position_t));
}

void hash_table_init(hash_table_t *table) {
  hash_options_t default_options = {
      .size = 1 << 10,
      .hasher = knuth_hash,
      .strategy = PROBE_LINEAR,
  };
  return hash_table_init_ex(table, default_options);
}

size_t linear_probe(hash_table_t *table, int hash, size_t i) {
  return (hash + i) % table->size;
}

// "Introduction to Algorithms, third edition", Cormen et al., 13.3.2 p:272
// Assuming that the table size is always a power of two, we modify the second
// hash to always return an odd number, such that the second hash and the table
// size are coprime to each other. That guarantees that the whole table will be
// searched.
size_t double_hash(hash_table_t *table, int hash, const char *key, size_t i) {
  assert(table->double_hasher &&
         "To use double hash, you must provide a second hash function");
  int second_hash =
      table->double_hasher(table, key) | 1; // Always use an odd second hash
  return (hash + (i * second_hash)) % table->size;
}

size_t probe(hash_table_t *table, int hash, const char *key, size_t i) {
  switch (table->strategy) {
  case PROBE_LINEAR:
    return linear_probe(table, hash, i);
  case PROBE_DOUBLE_HASH:
    return double_hash(table, hash, key, i);
  default:
    assert(0 && "not implemented");
  }
}

void hash_table_insert(hash_table_t *table, char *key, void *value) {
  int hash = table->hasher(table, key);
  bool found = false;

  for (size_t i = 0; i < table->size; i++) {
    size_t idx = probe(table, hash, key, i);
    hash_position_t *position = &table->values[idx];
    if (!position->in_use) {
      position->in_use = true;
      position->key = key;
      position->value = value;
      found = true;
      break;
    }
  }

  if (!found) {
    assert(0 && "TODO: rehash table");
  }
}

hash_position_t *hash_table_lookup_internal(hash_table_t *table,
                                            const char *key) {
  int hash = table->hasher(table, key);

  for (size_t i = 0; i < table->size; i++) {
    size_t idx = probe(table, hash, key, i);
    hash_position_t *current = &table->values[idx];
    if (!current->in_use) {
      continue;
    }

    if (strcmp(current->key, key) == 0) {
      return current;
    }
  }

  return NULL;
}

void *hash_table_lookup(hash_table_t *table, const char *key) {
  hash_position_t *val = hash_table_lookup_internal(table, key);
  if (val) {
    return val->value;
  }

  return NULL;
}

void hash_table_delete(hash_table_t *table, const char *key) {
  hash_position_t *node = hash_table_lookup_internal(table, key);
  if (!node) {
    return;
  }

  node->in_use = false;
  HASH_TABLE_FREE(node->value);
  node->value = NULL;
  node->key = 0;
}

#endif // HASH_TABLE_IMPLEMENTATION
