
all: zad1

test: all
	./zad1 3 block big.ascii.pgm big_out.pgm
	./zad1 3 numbers small.ascii.pgm small_out.pgm

create_times: zad1
	echo "" > Times2.txt
	echo "Small image with mode BLOCK\n" >> Times2.txt
	./zad1 1 block small.ascii.pgm out.pgm
	./zad1 2 block small.ascii.pgm out.pgm
	./zad1 4 block small.ascii.pgm out.pgm
	./zad1 8 block small.ascii.pgm out.pgm
	./zad1 16 block small.ascii.pgm out.pgm
	echo "Big image with mode BLOCK\n" >> Times2.txt
	./zad1 1 block big.ascii.pgm out.pgm
	./zad1 2 block big.ascii.pgm out.pgm
	./zad1 4 block big.ascii.pgm out.pgm
	./zad1 8 block big.ascii.pgm out.pgm
	./zad1 16 block big.ascii.pgm out.pgm
	echo "Small image with mode NUMBERS\n" >> Times2.txt
	./zad1 1 numbers small.ascii.pgm out.pgm
	./zad1 2 numbers small.ascii.pgm out.pgm
	./zad1 4 numbers small.ascii.pgm out.pgm
	./zad1 8 numbers small.ascii.pgm out.pgm
	./zad1 16 numbers small.ascii.pgm out.pgm
	echo "Big image with mode NUMBERS\n" >> Times2.txt
	./zad1 1 numbers big.ascii.pgm out.pgm
	./zad1 2 numbers big.ascii.pgm out.pgm
	./zad1 4 numbers big.ascii.pgm out.pgm
	./zad1 8 numbers big.ascii.pgm out.pgm
	./zad1 16 numbers big.ascii.pgm out.pgm
	echo "\nKomentarz: \nW trybie NUMBERS wyniki czasow sa wyzsze niz w trybie BLOCK.\nProgram trybu NUMBERS wymaga znalezienia odpowiednich \nwartosci w calej macierzy dla kazdego watku,\ndlatego dane nie sa rozlozone rownomiernie na watki i dziala dluzej niz w trybie block.\n" >> Times2.txt


zad1:
	gcc -Wall zad1.c -o zad1 -pthread

clean:
	rm -f zad1 out.pgm big_out.pgm small_out.pgm Times2.txt