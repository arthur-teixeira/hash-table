#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (* hasher)(size_t, const char *);

typedef struct hash_table_t {
    hasher hasher;
    size_t size;
    int *bucket;
} hash_table_t;

int default_hasher(size_t table_size, const char *key) {
    int acc = 1;
    for (size_t i = 0; i < strlen(key); i++) {
        acc *=key[i];
    }

    return acc % table_size;
}

void hash_table_init(hash_table_t *table, size_t initial_size) {
    table->hasher = default_hasher;
    table->size = initial_size;
    table->bucket = calloc(initial_size, sizeof(int));
    memset(table->bucket, 0, initial_size * sizeof(int));
}

void hash_table_insert(hash_table_t *table, char *key, int value) {
    int hash = table->hasher(table->size, key);
    table->bucket[hash] = value;
}

int *hash_table_lookup(hash_table_t *table, const char *key) {
    int hash = table->hasher(table->size, key);
    return &table->bucket[hash];
}

int main() {
    printf("Hello world!\n");

    hash_table_t table;
    hash_table_init(&table, 10);
    hash_table_insert(&table, "hey", 10);

    int *val = hash_table_lookup(&table, "hey");
    printf("The value of key %s is %d\n", "hey", *val);
    return 0;
}
