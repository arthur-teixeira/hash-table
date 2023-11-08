#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef size_t (* hasher)(void *, const char *);

typedef struct node_t {
    struct node_t *next;
    struct node_t *prev;
    const char *key;
    int value;
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
        acc *=key[i] + i;
    }

    return acc; 
}

// "Introduction to Algorithms, third edition", Cormen et al., 13.3.2 p:263
// "The Art of Computer Programming, Volume 3, Sorting and Searching", D.E. Knuth, 6.4 p:516
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
    table->bucket = calloc(initial_size, sizeof(node_t *));
    memset(table->bucket, 0, initial_size * sizeof(node_t *));
}

void hash_table_insert(hash_table_t *table, char *key, int value) {
    int hash = table->hasher(table, key);
    node_t *head = table->bucket[hash];
    node_t *new_node = malloc(sizeof(node_t));
    *new_node = (node_t) {
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

int *hash_table_lookup(hash_table_t *table, const char *key) {
    int hash = table->hasher(table, key);
    node_t *head = table->bucket[hash];
    node_t *current = head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return &current->value;
        }
        current = current->next;
    }

    return NULL;
}

int main() {
    srand(time(NULL));
    printf("Hello world!\n");

    hash_table_t table;
    hash_table_init(&table, 10);
    hash_table_insert(&table, "hey", 10);
    hash_table_insert(&table, "yeh", 20);
    hash_table_insert(&table, "hello", 35);

    int *val = hash_table_lookup(&table, "hey");
    printf("The value of key %s is %d\n", "hey", *val);

    int *val2 = hash_table_lookup(&table, "yeh");
    printf("The value of key %s is %d\n", "yeh", *val2);

    int *val3 = hash_table_lookup(&table, "hello");
    printf("The value of key %s is %d\n", "hello", *val3);

    int *val4 = hash_table_lookup(&table, "non_existent");
    if (val4) {
        printf("The value of key %s is %d\n", "non_existent", *val4);
    } else {
        printf("The value of key %s was not found\n", "non_existent");
    }

    return 0;
}
