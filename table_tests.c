#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unity/unity.h>
#include <unity/unity_internals.h>
#define HASH_TABLE_IMPLEMENTATION
#include "stb_hashtable.h"

#define LEN(xs) sizeof(xs) / sizeof(xs[0])
hash_table_t sut;

void setUp(void) { hash_table_init(&sut); }

void tearDown(void) { memset(&sut, 0, sizeof(hash_table_t)); }

void populate(char **keys, size_t n) {
  for (size_t i = 0; i < n; i++) {
    int *val = malloc(sizeof(int));
    *val = i;
    hash_table_insert(&sut, keys[i], strlen(keys[i]), val);
  }
}

void assert_values(char **keys, size_t n, bool can_free) {
  for (size_t i = 0; i < n; i++) {
    int *val = hash_table_lookup(&sut, keys[i], strlen(keys[i]));
    TEST_ASSERT_EQUAL(*val, i);
    if (can_free) {
      free(val);
    }
  }
}

void test_insertions() {
  char *keys[] = {
      "hello",
      "hey",
      "olleh",
  };

  populate(keys, LEN(keys));
  assert_values(keys, LEN(keys), true);
}

// NOTE: This test is highly coupled to the hashing function
void test_collisions() {
  tearDown();
  hash_options_t opts = {
      .strategy = PROBE_LINEAR,
      .size = 2,
      .hasher = NULL,
      .comparer = memcmp_comparer,
  };
  hash_table_init_ex(&sut, opts);

  char *keys[] = {
      "hello",
      "helo",
  };

  populate(keys, LEN(keys));
  assert_values(keys, LEN(keys), true);
}

void test_double_hash_collisions() {
  tearDown();
  hash_options_t opts = {
      .strategy = PROBE_DOUBLE_HASH,
      .size = 2,
      .hasher = NULL,
      .double_hasher = NULL,
      .comparer = memcmp_comparer,
  };

  hash_table_init_ex(&sut, opts);

  char *keys[] = {
      "hello",
      "helo",
  };

  populate(keys, LEN(keys));
  assert_values(keys, LEN(keys), true);
}

void test_deletions() {
  char *keys[] = {
      "hello",
      "hey",
      "olleh",
  };

  populate(keys, LEN(keys));
  assert_values(keys, LEN(keys), false);

  for (size_t i = 0; i < LEN(keys); i++) {
    hash_table_delete(&sut, keys[i], strlen(keys[i]));
  }

  for (size_t i = 0; i < LEN(keys); i++) {
    int *val = hash_table_lookup(&sut, keys[i], strlen(keys[i]));
    TEST_ASSERT_NULL(val);
  }
}

void test_rehash() {
  tearDown();
  hash_options_t opts = {
      .strategy = PROBE_LINEAR,
      .size = 2,
      .hasher = NULL,
      .double_hasher = NULL,
      .comparer = memcmp_comparer,
  };

  hash_table_init_ex(&sut, opts);

  char *keys[] = {
      "hello", "hey", "hi", "bye", "byebye",
  };

  populate(keys, LEN(keys));
  assert_values(keys, LEN(keys), true);
  TEST_ASSERT_EQUAL(8, sut.size);
  TEST_ASSERT_EQUAL(5, sut.used);
}

typedef struct {
  int a;
  int b;
} key_type_a;

typedef struct {
  int a;
  int b;
  int c;
} key_type_b;

bool key_type_a_comparer(const void *a, size_t a_len, const void *b,
                         size_t b_len) {
  const key_type_a *ka = a;
  const key_type_a *kb = b;

  return a_len == b_len && ka->a == kb->a && ka->b == kb->b;
}

bool key_type_b_comparer(const void *a, size_t a_len, const void *b,
                         size_t b_len) {
  const key_type_b *ka = a;
  const key_type_b *kb = b;

  return a_len == b_len && ka->a == kb->a && ka->b == kb->b && ka->c == kb->c;
}

void test_other_key_types_a() {
  tearDown();
  hash_options_t opts = {
      .strategy = PROBE_LINEAR,
      .size = 2,
      .hasher = NULL,
      .comparer = key_type_a_comparer,
  };
  hash_table_init_ex(&sut, opts);

  key_type_a a_keys[] = {
      {1, 2},
      {3, 4},
      {5, 6},
  };

  for (size_t i = 0; i < LEN(a_keys); i++) {
    int *val = malloc(sizeof(int));
    *val = i;
    hash_table_insert(&sut, &a_keys[i], sizeof(key_type_a), val);
  }

  for (size_t i = 0; i < LEN(a_keys); i++) {
    int *val = hash_table_lookup(&sut, &a_keys[i], sizeof(key_type_a));
    TEST_ASSERT_EQUAL(*val, i);
    free(val);
  }
}

void test_other_key_types_b() {
  tearDown();
  hash_options_t opts = {
      .strategy = PROBE_LINEAR,
      .size = 2,
      .hasher = NULL,
      .comparer = key_type_b_comparer,
  };

  hash_table_init_ex(&sut, opts);

  key_type_b b_keys[] = {
      {1, 2, 3},
      {4, 5, 6},
      {7, 8, 9},
  };

  for (size_t i = 0; i < LEN(b_keys); i++) {
    int *val = malloc(sizeof(int));
    *val = i;
    hash_table_insert(&sut, &b_keys[i], sizeof(key_type_b), val);
  }

  for (size_t i = 0; i < LEN(b_keys); i++) {
    int *val = hash_table_lookup(&sut, &b_keys[i], sizeof(key_type_b));
    TEST_ASSERT_EQUAL(*val, i);
    free(val);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_insertions);
  RUN_TEST(test_collisions);
  RUN_TEST(test_deletions);
  RUN_TEST(test_double_hash_collisions);
  RUN_TEST(test_rehash);
  RUN_TEST(test_other_key_types_a);
  RUN_TEST(test_other_key_types_b);
  return UNITY_END();
}
