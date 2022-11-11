
all: zad3v1 zad3v2

test: testv1 testv2

testv2: zad3_v2
	./zad3_v2 ./test_directory

zad3v2:
	gcc -Wall zad3_v2.c -o zad3_v2


testv1: zad3_v1
	./zad3_v1 ./test_directory

zad3v1:
	gcc -Wall zad3_v1.c -o zad3_v1

clean:
	rm zad3_v1 zad3_v2