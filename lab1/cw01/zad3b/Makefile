ONE_FILE= medium.txt
TWO_FILEs= medium.txt big.txt
THREE_FILES= small.txt medium.txt big.txt
TEN_FILES= medium.txt medium2.txt medium3.txt medium4.txt medium5.txt medium6.txt medium7.txt medium8.txt medium9.txt medium10.txt

all_O0: zad2_dynamicO0 zad2_sharedO0 zad2_statO0
all_Os: zad2_dynamicOs zad2_sharedOs zad2_statOs
all_O3: zad2_dynamicO3 zad2_sharedO3 zad2_statO3

test_zad3b:
	echo "Zadanie 3b" > results3b.txt
	make zad3b0
	make zad3bs
	make zad3b3
	echo "" >> results3b.txt
	echo "Komentarz: " >> results3b.txt
	echo "Wyniki sa bardzo podobne do siebie" >> results3b.txt
	echo "Czas wykonania programu nie rozni sie ze wzgledu na sposob optymalizacji." >> results3b.txt

zad3b0: zad2_dynamicO0 zad2_sharedO0 zad2_statO0
	./zad2_dll create_table 2 >> results3b.txt
	./zad2_dll create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_dll create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_dll create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_dll create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

	./zad2_shared create_table 2 >> results3b.txt
	./zad2_shared create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_shared create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_shared create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_shared create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

	./zad2_stat create_table 2 >> results3b.txt
	./zad2_stat create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_stat create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_stat create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_stat create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

zad2_dynamicO0: shared_libO0
	gcc -Wall -O0 -o zad2_dll zad2_dll.c -ldl

zad2_sharedO0: shared_libO0
	gcc -O0 -Wall -o zad2_shared zad2.c -L. -lmemory_block_operations -Wl,-rpath=`pwd`

shared_libO0:
	gcc -O0 -Wall -fPIC -pedantic memory_block_operations.c -c -o memory_block_operations.o
	gcc -O0 -Wall -shared -fPIC -o libmemory_block_operations.so memory_block_operations.o

zad2_statO0: stat_libO0
	gcc -Wall -O0 -o zad2_stat zad2.c -L . -lmemory_block_operationsS

stat_libO0:
	gcc -O0 -Wall -pedantic memory_block_operations.c -c -o memory_block_operations.o
	ar crs libmemory_block_operationsS.a memory_block_operations.o

zad3bs: zad2_dynamicOs zad2_sharedOs zad2_statOs
	./zad2_dll create_table 2 >> results3b.txt
	./zad2_dll create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_dll create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_dll create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_dll create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

	./zad2_shared create_table 2 >> results3b.txt
	./zad2_shared create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_shared create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_shared create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_shared create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

	./zad2_stat create_table 2 >> results3b.txt
	./zad2_stat create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_stat create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_stat create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_stat create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

zad2_dynamicOs: shared_libOs
	gcc -Wall -Os -o zad2_dll zad2_dll.c -ldl

zad2_sharedOs: shared_libOs
	gcc -Os -Wall -o zad2_shared zad2.c -L. -lmemory_block_operations -Wl,-rpath=`pwd`

shared_libOs:
	gcc -Os -Wall -fPIC -pedantic memory_block_operations.c -c -o memory_block_operations.o
	gcc -Os -Wall -shared -fPIC -o libmemory_block_operations.so memory_block_operations.o

zad2_statOs: stat_libOs
	gcc -Wall -Os -o zad2_stat zad2.c -L . -lmemory_block_operationsS

stat_libOs:
	gcc -Os -Wall -pedantic memory_block_operations.c -c -o memory_block_operations.o
	ar crs libmemory_block_operationsS.a memory_block_operations.o

zad3b3: zad2_dynamicO3 zad2_sharedO3 zad2_statO3
	./zad2_dll create_table 2 >> results3b.txt
	./zad2_dll create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_dll create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_dll create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_dll create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

	./zad2_shared create_table 2 >> results3b.txt
	./zad2_shared create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_shared create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_shared create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_shared create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

	./zad2_stat create_table 2 >> results3b.txt
	./zad2_stat create_table 1 wc_files $(ONE_FILE) >> results3b.txt
	./zad2_stat create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_stat create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3b.txt
	./zad2_stat create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3b.txt

zad2_dynamicO3: shared_libO3
	gcc -Wall -O3 -o zad2_dll zad2_dll.c -ldl

zad2_sharedO3: shared_libO3
	gcc -O3 -Wall -o zad2_shared zad2.c -L. -lmemory_block_operations -Wl,-rpath=`pwd`

shared_libO3:
	gcc -O3 -Wall -fPIC -pedantic memory_block_operations.c -c -o memory_block_operations.o
	gcc -O3 -Wall -shared -fPIC -o libmemory_block_operations.so memory_block_operations.o

zad2_statO3: stat_libO3
	gcc -Wall -O3 -o zad2_stat zad2.c -L . -lmemory_block_operationsS

stat_libO3:
	gcc -Wall -O3 -Wall -pedantic memory_block_operations.c -c -o memory_block_operations.o
	ar crs libmemory_block_operationsS.a memory_block_operations.o

clean:
	rm *.o *.a *.so zad2_shared zad2_stat zad2_dll
