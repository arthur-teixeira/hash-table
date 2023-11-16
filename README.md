# Hash table

A simple [stb style](https://github.com/nothings/stb) hash table for c/c++. It uses open addressing and linear probing to manage collisions.

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

## TO DO
- [X] Add option to change the hashing function
- [X] Add option to change the probing strategy (linear, quadratic and double hash)
- [ ] Rehash table depending on the load factor
- [ ] Universal hashing function
- [ ] Add possibility for the key to be any type, not just strings
