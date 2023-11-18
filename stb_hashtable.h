#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef size_t (*hasher_t)(void *, const void *, size_t);
typedef bool (*comparer_t)(const void *, size_t, const void *, size_t);

typedef struct hash_position_t {
  bool in_use;
  const void *key;
  size_t key_len;
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
  comparer_t comparer;
  probe_strategy strategy;
  size_t size;
  size_t used;
} hash_options_t;

typedef struct hash_table_t {
  probe_strategy strategy;
  hasher_t hasher;
  comparer_t comparer;
  hasher_t double_hasher;
  size_t size;
  size_t p;
  hash_position_t *values;
  size_t used;
} hash_table_t;

void *hash_table_lookup(hash_table_t *table, const void *key, size_t key_len);
void hash_table_delete(hash_table_t *table, const void *key, size_t key_len);
void hash_table_insert(hash_table_t *table, const void *key, size_t key_len,
                       void *value);
void hash_table_init(hash_table_t *table);
void hash_table_init_ex(hash_table_t *table, hash_options_t options);

bool memcmp_comparer(const void *a, size_t a_len, const void *b, size_t b_len);

#endif // HASH_TABLE_H

#ifdef HASH_TABLE_IMPLEMENTATION

#ifndef HASH_TABLE_MALLOC
#define HASH_TABLE_MALLOC(nmemb) malloc(nmemb)
#endif

#ifndef HASH_TABLE_REALLOC
#define HASH_TABLE_REALLOC(ptr, nmemb) realloc(ptr, nmemb)
#endif

#ifndef HASH_TABLE_FREE
#define HASH_TABLE_FREE(ptr) free(ptr)
#endif

#ifndef HASH_TABLE_LOAD_FACTOR_THRESHOLD
#define HASH_TABLE_LOAD_FACTOR_THRESHOLD 0.65f
#endif

int buf_as_int(const void *key, size_t size) {
  const uint8_t *s = (const uint8_t *)key;
  int acc = 1;
  for (size_t i = 0; i < size; i++) {
    acc *= s[i] + i;
  }

  return acc;
}

// "Introduction to Algorithms, third edition", Cormen et al., 13.3.2 p:263
// "The Art of Computer Programming, Volume 3, Sorting and Searching", D.E.
// Knuth, 6.4 p:516
size_t knuth_hash(void *t, const void *key, size_t len) {
  hash_table_t *table = t;
  size_t knuth = 2654435769L;
  size_t key_as_int = buf_as_int(key, len);
  size_t hash = (key_as_int * knuth) >> (32 - table->p);

  return hash % table->size;
}

// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
size_t fnv_hash(void *t, const void *key, size_t len) {
  hash_table_t *table = t;
  size_t fnv_prime = 1099511628211U;
  size_t fnv_offset = 14695981039346656037U;

  size_t hash = fnv_offset;

  const uint8_t *const s = (const uint8_t *)key;

  for (size_t i = 0; i < len; i++) {
    hash *= fnv_prime;
    hash ^= s[i];
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
  table->comparer = options.comparer;
  memset(table->values, 0, options.size * sizeof(hash_position_t));
}

bool memcmp_comparer(const void *a, size_t a_len, const void *b, size_t b_len) {
  return a_len == b_len && (memcmp(a, b, a_len) == 0);
}

void hash_table_init(hash_table_t *table) {
  hash_options_t default_options = {
      .hasher = knuth_hash,
      .comparer = memcmp_comparer,
      .strategy = PROBE_LINEAR,
      .size = 1 << 10,
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
size_t double_hash(hash_table_t *table, int hash, const char *key,
                   size_t key_len, size_t i) {
  assert(table->double_hasher &&
         "To use double hash, you must provide a second hash function");
  int second_hash = table->double_hasher(table, key, key_len) |
                    1; // Always use an odd second hash
  return (hash + (i * second_hash)) % table->size;
}

size_t probe(hash_table_t *table, int hash, const char *key, size_t key_len,
             size_t i) {
  switch (table->strategy) {
  case PROBE_LINEAR:
    return linear_probe(table, hash, i);
  case PROBE_DOUBLE_HASH:
    return double_hash(table, hash, key, key_len, i);
  default:
    assert(0 && "not implemented");
  }
}

double load_factor(hash_table_t *t) {
  return (double)t->used / (double)t->size;
}

void rehash(hash_table_t *t) {
  hash_position_t *old_values = t->values;
  size_t old_size = t->size;
  t->size *= 2;
  t->used = 0;

  t->values = HASH_TABLE_MALLOC(t->size * sizeof(hash_position_t));
  memset(t->values, 0, t->size * sizeof(hash_position_t));

  for (size_t i = 0; i < old_size; i++) {
    if (old_values[i].in_use) {
      hash_table_insert(t, old_values[i].key, old_values[i].key_len,
                        old_values[i].value);
    }
  }

  HASH_TABLE_FREE(old_values);
}

void hash_table_insert(hash_table_t *table, const void *key, size_t key_len,
                       void *value) {
  if (load_factor(table) > HASH_TABLE_LOAD_FACTOR_THRESHOLD) {
    rehash(table);
  }

  int hash = table->hasher(table, key, key_len);
  for (size_t i = 0; i < table->size; i++) {
    size_t idx = probe(table, hash, key, key_len, i);
    hash_position_t *position = &table->values[idx];
    if (!position->in_use) {
      position->in_use = true;
      position->key = key;
      position->key_len = key_len;
      position->value = value;
      table->used++;
      break;
    }
  }
}

hash_position_t *hash_table_lookup_internal(hash_table_t *table,
                                            const void *key, size_t key_len) {
  int hash = table->hasher(table, key, key_len);

  for (size_t i = 0; i < table->size; i++) {
    size_t idx = probe(table, hash, key, key_len, i);
    hash_position_t *current = &table->values[idx];
    if (!current->in_use) {
      continue;
    }

    if (table->comparer(current->key, current->key_len, key, key_len)) {
      return current;
    }
  }

  return NULL;
}

void *hash_table_lookup(hash_table_t *table, const void *key, size_t key_len) {
  hash_position_t *val = hash_table_lookup_internal(table, key, key_len);
  if (val) {
    return val->value;
  }

  return NULL;
}

void hash_table_delete(hash_table_t *table, const void *key, size_t key_len) {
  hash_position_t *node = hash_table_lookup_internal(table, key, key_len);
  if (!node) {
    return;
  }

  node->in_use = false;
  HASH_TABLE_FREE(node->value);
  node->value = NULL;
  node->key = 0;
  table->used--;
}

#endif // HASH_TABLE_IMPLEMENTATION
