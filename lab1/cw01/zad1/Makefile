
all: shared_lib stat_lib

shared_lib:
	gcc -Wall -fPIC -pedantic memory_block_operations.c -c -o memory_block_operations.o
	gcc -Wall -shared -fPIC -o libmemory_block_operations.so memory_block_operations.o

stat_lib:
	gcc -Wall -pedantic memory_block_operations.c -c -o memory_block_operations.o
	ar crs libmemory_block_operationsS.a memory_block_operations.o

clean:
	rm *.a *.so *.o
