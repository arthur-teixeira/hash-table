#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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
    hash_table_insert(&sut, keys[i], val);
  }
}

void assert_values(char **keys, size_t n, bool can_free) {
  for (size_t i = 0; i < n; i++) {
    int *val = hash_table_lookup(&sut, keys[i]);
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
    hash_table_delete(&sut, keys[i]);
  }

  for (size_t i = 0; i < LEN(keys); i++) {
    int *val = hash_table_lookup(&sut, keys[i]);
    TEST_ASSERT_NULL(val);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_insertions);
  RUN_TEST(test_collisions);
  RUN_TEST(test_deletions);
  RUN_TEST(test_double_hash_collisions);
  return UNITY_END();
}
