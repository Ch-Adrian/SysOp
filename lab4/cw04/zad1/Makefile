
all: zad1 zad1e

test: zad1 zad1e
	echo "Parametr ignore:" > raport2.txt
	./zad1 ignore >> raport2.txt
	echo "" >> raport2.txt
	echo "Dla parametru ignore:\n- program zad1 ignoruje sygnal SIGUSR1\n- proces potomny dziedziczy ignorowanie i tez ignoruje sygnal SIGUSR1\n- program wywolany funkcją execv tez dziedziczy ignorowanie sygnalu SIGUSR1\n" >> raport2.txt
	echo "Parametr handler:" >> raport2.txt
	./zad1 handler >> raport2.txt
	echo "" >> raport2.txt
	echo "Dla parametru handler:\n- program zad1 wywoluje funkcje podana jako handler\n- proces potomny dziedziczy ta dyspozycje i rowniez wywoluje funkcje podana jako handler\n" >> raport2.txt
	echo "Parametr mask:" >> raport2.txt
	./zad1 mask >> raport2.txt
	echo "" >> raport2.txt
	echo "Dla paramteru mask:\n- program zad1 blokuje sygnal SIGUSR1\n- proces potomny dziedziczy ta dyspozycje i sygnal jest blokowny\n- program wywolany funkcja execv rowniez dziedziczy ta dyspozycje i sygnal jest blokowany\n" >> raport2.txt
	echo "Parametr pending:" >> raport2.txt
	./zad1 pending >> raport2.txt
	echo "" >> raport2.txt
	echo "Dla parametru pending:\n- program zad1 blokuje sygnal i widzi, ze sygnal oczekuje\n- proces potomny nie widzi, ze sygnal oczekuje u przodka\n- program wywolany funkcja execv widzi, ze sygnal oczekuje u przodka\n" >> raport2.txt

test_ignore: zad1 zad1e
	./zad1 ignore

test_handler: zad1 zad1e
	./zad1 handler

test_mask: zad1 zad1e
	./zad1 mask

test_pending: zad1 zad1e
	./zad1 pending

zad1e:
	gcc -Wall zad1e.c -o zad1e

zad1:
	gcc -Wall zad1.c -o zad1

clean:
	rm zad1 zad1e