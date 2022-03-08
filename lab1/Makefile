
SRC_FILES= source/memory_block_operations.c
OBJ_FILES= source/memory_block_operations.o
SRC_TEST_FILES= source/main_fun.c

run_test: test1
	./test

test1: main
	gcc -Wall -pedantic -fsanitize=address $(SRC_TEST_FILES) $(OBJ_FILES) -o test

main:
	gcc -Wall -O2 -pedantic -fsanitize=address $(SRC_FILES) -c -o $(OBJ_FILES)

clean:
	rm prg
