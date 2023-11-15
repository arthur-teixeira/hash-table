# Hash table

A simple stb style hash table for c/c++. It uses open addressing and linear probing to manage collisions.

## How to use
- Copy stb_hashtable.h to your project
- You can use it as a normal header wherever you want to.
- To actually include the implementation, you have to define the HASH_TABLE_IMPLEMENTATION macro before the include:

```c
#include <blah.h>
#include <blahblah.h>

#define HASH_TABLE_IMPLEMENTATION
#include "stb_hashtable.h"
```

## Running tests
To run the tests, you have to install the [Unity testing framework](https://github.com/ThrowTheSwitch/Unity).

```sh
$ ./run-tests.sh
```
