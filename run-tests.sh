CFLAGS="-Wall -Wextra -ggdb" 

mkdir -p ./bin/

clang $CFLAGS -o ./bin/tests  ./*.c -lunity

./bin/tests
