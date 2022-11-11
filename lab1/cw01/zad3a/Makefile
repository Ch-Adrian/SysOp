ONE_FILE= medium.txt
TWO_FILEs= medium.txt big.txt
THREE_FILES= small.txt medium.txt big.txt
TEN_FILES= medium.txt medium2.txt medium3.txt medium4.txt medium5.txt medium6.txt medium7.txt medium8.txt medium9.txt medium10.txt


all:  zad2_dynamic zad2_shared zad2_stat

test: zad2_dynamic zad2_shared zad2_stat
	echo "Zadanie 3a" > results3a.txt
	 ./zad2_dll create_table 2 >> results3a.txt
	./zad2_dll create_table 1 wc_files $(ONE_FILE) >> results3a.txt
	./zad2_dll create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3a.txt
	./zad2_dll create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3a.txt
	./zad2_dll create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3a.txt

	./zad2_shared create_table 2 >> results3a.txt
	./zad2_shared create_table 1 wc_files $(ONE_FILE) >> results3a.txt
	./zad2_shared create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3a.txt
	./zad2_shared create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3a.txt
	./zad2_shared create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3a.txt

	./zad2_stat create_table 2 >> results3a.txt
	./zad2_stat create_table 1 wc_files $(ONE_FILE) >> results3a.txt
	./zad2_stat create_table 10 wc_files $(ONE_FILE) remove_block 1 >> results3a.txt
	./zad2_stat create_table 2 wc_files $(ONE_FILE) remove_block 1 >> results3a.txt
	./zad2_stat create_table 3 wc_files $(TWO_FILES) wc_files $(ONE_FILE) remove_block 1 remove_block 2 >> results3a.txt

	echo "" >> results3a.txt
	echo "Komentarz: " >> results3a.txt
	echo "Wyniki sa bardzo podobne do siebie" >> results3a.txt
	echo "Czas wykonania programu nie rozni się ze wzgledu na sposob dołaczenia biblioteki." >> results3a.txt

zad2_dynamic: shared_lib
	gcc -Wall -o zad2_dll zad2_dll.c -ldl

zad2_shared: shared_lib
	gcc -Wall -o zad2_shared zad2.c -L. -lmemory_block_operations -Wl,-rpath=`pwd`

shared_lib:
	gcc -Wall -fPIC -pedantic memory_block_operations.c -c -o memory_block_operations.o
	gcc -Wall -shared -fPIC -o libmemory_block_operations.so memory_block_operations.o

zad2_stat: stat_lib
	gcc -Wall -o zad2_stat zad2.c -L . -lmemory_block_operationsS

stat_lib:
	gcc -Wall -pedantic memory_block_operations.c -c -o memory_block_operations.o
	ar crs libmemory_block_operationsS.a memory_block_operations.o

clean:
	rm *.o *.a *.so zad2_shared zad2_stat zad2_dll
