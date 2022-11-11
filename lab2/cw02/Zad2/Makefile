
all: zad2_lib zad2_sys

test: zad2_sys zad2_lib
	echo "Pomiar programu zad2_sys\n" > pomiar_zad_2.txt
	echo "Wywolana komenda: ./zad2_sys a big.txt\n" >> pomiar_zad_2.txt
	./zad2_sys a big.txt >> pomiar_zad_2.txt
	echo "Wywolana komenda: ./zad2_sys a small.txt\n" >> pomiar_zad_2.txt
	./zad2_sys a small.txt >> pomiar_zad_2.txt
	echo "Pomiar programu zad2_lib\n" >> pomiar_zad_2.txt
	echo "Wywolana komenda: ./zad2_lib a big.txt\n" >> pomiar_zad_2.txt
	./zad2_lib a big.txt >> pomiar_zad_2.txt
	echo "Wywolana komenda: ./zad2_lib a small.txt\n" >> pomiar_zad_2.txt
	./zad2_lib a small.txt >> pomiar_zad_2.txt

test_sys: zad2_sys
	./zad2_sys a big.txt

test_lib: zad2_lib
	./zad2_lib a big.txt

zad2_sys:
	gcc -Wall zad2_sys.c -o zad2_sys

zad2_lib:
	gcc -Wall zad2_lib.c -o zad2_lib

clean:
	rm zad2_lib zad2_sys
