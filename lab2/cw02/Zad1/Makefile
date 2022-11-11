
all: zad1_sys zad1_lib

test: zad1_sys zad1_lib
	echo "Pomiar programu zad1_sys\n" > pomiar_zad_1.txt
	echo "Wywolana komenda: ./zad1_sys big.txt fdest.txt\n" >> pomiar_zad_1.txt
	./zad1_sys big.txt fdest.txt >> pomiar_zad_1.txt
	echo "Wywolana komenda: ./zad1_sys small.txt fdest.txt\n" >> pomiar_zad_1.txt
	./zad1_sys small.txt fdest.txt >> pomiar_zad_1.txt
	echo "Pomiar programu zad1_lib\n" >> pomiar_zad_1.txt
	echo "Wywolana komenda: ./zad1_lib big.txt fdest.txt\n" >> pomiar_zad_1.txt
	./zad1_lib big.txt fdest.txt >> pomiar_zad_1.txt
	echo "Wywolana komenda: ./zad1_lib small.txt fdest.txt\n" >> pomiar_zad_1.txt
	./zad1_lib small.txt fdest.txt >> pomiar_zad_1.txt

test_sys: zad1_sys
	./zad1_sys small.txt fdest.txt

test_sys2: zad1_sys
	./zad1_sys big.txt fdest.txt

zad1_sys:
	gcc -Wall zad1_sys.c -o zad1_sys


test_lib: zad1_lib
	./zad1_lib small.txt fdest.txt

test_lib2: zad1_lib
	./zad1_lib big.txt fdest.txt

zad1_lib: 
	gcc -Wall zad1_lib.c -o zad1_lib
	
clean:
	rm zad1_lib zad1_sys
