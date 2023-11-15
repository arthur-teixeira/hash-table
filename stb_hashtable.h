#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef size_t (*hasher)(void *, const char *);

typedef struct hash_position_t {
  bool in_use;
  const char *key;
  void *value;
} hash_position_t;

typedef struct hash_table_t {
  hasher hasher;
  size_t size;
  size_t p;
  hash_position_t *values;
} hash_table_t;

void hash_table_delete(hash_table_t *table, const char *key);
void *hash_table_lookup(hash_table_t *table, const char *key);
void hash_table_insert(hash_table_t *table, char *key, void *value);
void hash_table_init(hash_table_t *table);
void hash_table_init_ex(hash_table_t *table, size_t initial_size);

#endif // HASH_TABLE_H

#define HASH_TABLE_IMPLEMENTATION // REMOVE
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

void hash_table_init_ex(hash_table_t *table, size_t initial_size) {
  table->hasher = knuth_hash;
  table->p = rand() % 32;
  table->size = initial_size;
  table->values = HASH_TABLE_MALLOC(initial_size * sizeof(hash_position_t));
  memset(table->values, 0, initial_size * sizeof(hash_position_t));
}

void hash_table_init(hash_table_t *table) {
  return hash_table_init_ex(table, 1 << 10);
}

void hash_table_insert(hash_table_t *table, char *key, void *value) {
  int hash = table->hasher(table, key);
  bool found = false;

  // Linear probing the whole table
  for (size_t i = 0; i < table->size; i++) {
    hash_position_t *position = &table->values[(hash + i) % table->size];
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
    hash_position_t *current = &table->values[(hash + i) % table->size];
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
