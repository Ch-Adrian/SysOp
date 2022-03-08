
SRC_FILES= source/memory_block_operations.c
OBJ_FILES= source/memory_block_operations.o
STAT_LIB_FILE= library/libmemory_block_operations.a
SRC_TEST_FILES= source/main_fun.c

run_test1: test1
	./test

test1:
	make clean
	gcc -Wall -pedantic source/memory_block_operations.c -c -o source/memory_block_operations.o
	gcc -Wall -pedantic -fsanitize=address source/main_fun.c source/memory_block_operations.o -o test

# stat_test: stat_lib
# 	gcc -o test source/main_fun.c -L ./library -lmemory_block_operations

# shared_test: shared_lib
# 	gcc -o test source/main_fun.c -L ./library -lmemory_block_operations

shared_lib:
	make clean
	gcc -Wall -fPIC -pedantic source/memory_block_operations.c -c -o source/memory_block_operations.o
	gcc -Wall -shared -fPIC -o library/libmemory_block_operations.so source/memory_block_operations.o
	gcc -Wall -o test source/main_fun.c -L./library -lmemory_block_operations -Wl,-rpath=`pwd`

stat_lib:
	make clean
	gcc -Wall -pedantic source/memory_block_operations.c -c -o source/memory_block_operations.o
	ar crs library/libmemory_block_operations.a source/memory_block_operations.o
	gcc -o test source/main_fun.c -L ./library -lmemory_block_operations
# main:
# 	gcc -Wall -pedantic source/memory_block_operations.c -c -o source/memory_block_operations.o

clean:
	rm -f source/*.o library/*.a library/*.so test
