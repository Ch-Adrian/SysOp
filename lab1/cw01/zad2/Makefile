ONE_FILE= medium.txt
TWO_FILEs= medium.txt big.txt
THREE_FILES= small.txt medium.txt big.txt
TEN_FILES= medium.txt medium2.txt medium3.txt medium4.txt medium5.txt medium6.txt medium7.txt medium8.txt medium9.txt medium10.txt

all: zad2_shared zad2_dynamic zad2_stat

test_dynamic: zad2_dynamic
	./zad2_dll create_table 2
	./zad2_dll create_table 1 wc_files $(ONE_FILE)
	./zad2_dll create_table 10 wc_files $(ONE_FILE) remove_block 2
	./zad2_dll create_table 2 wc_files $(ONE_FILE) remove_block 1
	./zad2_dll create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2
	./zad2_dll create_table 3 wc_files $(TEN_FILES) wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 remove_block 3

test_shared: zad2_shared
	./zad2_shared create_table 2
	./zad2_shared create_table 1 wc_files $(ONE_FILE)
	./zad2_shared create_table 10 wc_files $(ONE_FILE) remove_block 2
	./zad2_shared create_table 2 wc_files $(ONE_FILE) remove_block 1
	./zad2_shared create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2
	./zad2_shared create_table 3 wc_files $(TEN_FILES) wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 remove_block 3

test_stat: zad2_stat
	./zad2_stat create_table 2
	./zad2_stat create_table 1 wc_files $(ONE_FILE)
	./zad2_stat create_table 10 wc_files $(ONE_FILE) remove_block 2
	./zad2_stat create_table 2 wc_files $(ONE_FILE) remove_block 1
	./zad2_stat create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2
	./zad2_stat create_table 3 wc_files $(TEN_FILES) wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 remove_block 3

zad2_dynamic: shared_lib
	gcc -o zad2_dll zad2_dll.c -ldl

zad2_shared: shared_lib
	gcc -Wall -o zad2_shared zad2.c -L. -lmemory_block_operations -Wl,-rpath=`pwd`

zad2_stat: stat_lib
	gcc -o zad2_stat zad2.c -L . -lmemory_block_operationsS

shared_lib:
	gcc -Wall -fPIC -pedantic memory_block_operations.c -c -o memory_block_operations.o
	gcc -Wall -shared -fPIC -o libmemory_block_operations.so memory_block_operations.o

stat_lib:
	gcc -Wall -pedantic memory_block_operations.c -c -o memory_block_operations.o
	ar crs libmemory_block_operationsS.a memory_block_operations.o

clean:
	rm *.o *.a *.so zad2_shared zad2_stat zad2_dll
