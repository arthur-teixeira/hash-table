#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef size_t (*hasher)(void *, const char *);

typedef struct node_t {
  struct node_t *next;
  struct node_t *prev;
  const char *key;
  void *value;
} node_t;

typedef struct hash_table_t {
  hasher hasher;
  size_t size;
  size_t p;
  node_t **bucket;
} hash_table_t;

int string_as_int(const char *key) {
  int acc = 1;
  for (size_t i = 0; i < strlen(key); i++) {
    acc *= key[i] + i;
  }

  return acc;
}

void hash_table_delete(hash_table_t *table, const char *key);
void *hash_table_lookup(hash_table_t *table, const char *key);
void hash_table_insert(hash_table_t *table, char *key, void *value);
void hash_table_init(hash_table_t *table, size_t initial_size);

#endif // HASH_TABLE_H

#ifdef HASH_TABLE_IMPLEMENTATION

#ifndef HASH_TABLE_MALLOC
#define HASH_TABLE_MALLOC(nmemb) malloc(nmemb)
#endif

#ifndef HASH_TABLE_FREE
#define HASH_TABLE_FREE(ptr) free(ptr)
#endif

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

void hash_table_init(hash_table_t *table, size_t initial_size) {
  table->hasher = knuth_hash;
  table->p = rand() % 32;
  table->size = initial_size;
  table->bucket = HASH_TABLE_MALLOC(initial_size * sizeof(node_t *));
  memset(table->bucket, 0, initial_size * sizeof(node_t *));
}

void hash_table_insert(hash_table_t *table, char *key, void *value) {
  int hash = table->hasher(table, key);
  node_t *head = table->bucket[hash];
  node_t *new_node = HASH_TABLE_MALLOC(sizeof(node_t));
  *new_node = (node_t){
      .key = key,
      .value = value,
      .next = head,
      .prev = NULL,
  };

  if (head) {
    head->next = new_node;
  } else {
    table->bucket[hash] = new_node;
  }
}

node_t *hash_table_lookup_internal(hash_table_t *table, const char *key) {
  int hash = table->hasher(table, key);
  node_t *head = table->bucket[hash];
  node_t *current = head;
  while (current) {
    if (strcmp(current->key, key) == 0) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

void *hash_table_lookup(hash_table_t *table, const char *key) {
  node_t *val = hash_table_lookup_internal(table, key);
  if (val) {
    return val->value;
  }

  return NULL;
}

void hash_table_delete(hash_table_t *table, const char *key) {
  node_t *node = hash_table_lookup_internal(table, key);
  if (!node) {
    return;
  }

  if (node->prev) {
    node->prev->next = node->next;
  }

  if (node->next) {
    node->next->prev = node->prev;
  }

  HASH_TABLE_FREE(node->value);
  HASH_TABLE_FREE(node);
}

#endif // HASH_TABLE_IMPLEMENTATION
