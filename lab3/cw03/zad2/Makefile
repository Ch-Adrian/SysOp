
all: zad2

test: zad2
	./zad2 0.001 13
	./zad2 0.1 5
	./zad2 0.00000001 1000
	./zad2 0.000000001 1000
	./zad2 0.000000001 3000
	./zad2 0.000000001 5000

test_zad2: zad2
	./zad2 0.01 13

zad2:
	gcc -Wall zad2.c -o zad2

clean:
	rm W*.txt zad2

clean_pomiary:
	> pomiary.txt