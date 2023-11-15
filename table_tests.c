#include <stdlib.h>
#include <unity/unity.h>
#define HASH_TABLE_IMPLEMENTATION
#include "stb_hashtable.h"

#define LEN(xs) sizeof(xs) / sizeof(xs[0])

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_insertions() {
  char *keys[] = {
      "hello",
      "hey",
      "olleh",
  };

  hash_table_t sut;
  hash_table_init(&sut);

  for (size_t i = 0; i < LEN(keys); i++) {
    int *val = malloc(sizeof(int));
    *val = i;
    hash_table_insert(&sut, keys[i], val);
  }

  for (size_t i = 0; i < LEN(keys); i++) {
    int *val = hash_table_lookup(&sut, keys[i]);
    TEST_ASSERT_EQUAL(*val, i);
    free(val);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_insertions);
  return UNITY_END();
}
