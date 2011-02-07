LIBS="-lz"

CC=clang++
$CC -Wall -c -O0 -g -fno-inline main.cpp nbt.cpp
$CC -o main.exe ${LIBS} nbt.o main.o

exit $?
