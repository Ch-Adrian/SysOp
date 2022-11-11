
all: Consumer Producer check_line

test5P: Consumer Producer check_line
	mkfifo -m 666 fifo_file10
	./Consumer fifo_file10 out_5P.txt 60 &
	./Producer fifo_file10 2 in1_s.txt 60 &
	./Producer fifo_file10 10 in3_s.txt 60 &
	./Producer fifo_file10 5 in2_s.txt 60 &
	./Producer fifo_file10 3 in1_s.txt 60 &
	./Producer fifo_file10 7 in2_s.txt 60 &
	sleep 20
	./check_line in1_s.txt 2 out_5P.txt
	./check_line in2_s.txt 5 out_5P.txt
	./check_line in3_s.txt 10 out_5P.txt
	./check_line in1_s.txt 3 out_5P.txt
	./check_line in2_s.txt 7 out_5P.txt

test:
	make create_fifo_files
	make test1
	make test2
	make test3
	make test4
	make test5
	make test6
	make test7
	make test8
	make test9
	sleep 20
	make check_test

check_test: check_line
	echo " 1 consumer: " > wnioski.txt
	make check_test1 >> wnioski.txt
	make check_test4 >> wnioski.txt
	make check_test7 >> wnioski.txt
	echo " N consumers: " >> wnioski.txt
	make check_test2 >> wnioski.txt
	make check_test3 >> wnioski.txt
	make check_test5 >> wnioski.txt
	make check_test6 >> wnioski.txt
	make check_test8 >> wnioski.txt
	make check_test9 >> wnioski.txt
	echo "" >> wnioski.txt
	echo "Correct oznacza poprawne wykonanie programu.\nProgram dla jednego konsumenta zwraca poprawne wyniki.\nGdy zwieksza sie liczba konsumentow program dziala niepoprawnie.\nKopiuje za malo danych lub wstawia dane w nieodpowiednie miejsca\nDla wartosci n < PIPE_BUF zapisywanie danych do kolejki jest wykonywane bezproblemowo,\ndlatego programy z wieloma producentami i jednym konsumentem dzialaja poprawnie.\nDla n>= PIPE_BUF nie jest gwarantowanie poprawne kopiowanie danych,\nwiec dla dużych wartości np. 6000( > PIPE_BUF) transport danych moze sie nie udac.\nDla linuksa wartość PIPE_BUF = 4096 " >> wnioski.txt

clean:
	rm -f Producer Consumer check_line
	make cleanTxt


test1: Consumer Producer
	./Consumer fifo_file1 out_NP_1K_s.txt 60 &
	./Producer fifo_file1 2 in1_s.txt 60 &
	./Producer fifo_file1 10 in3_s.txt 60 &
	./Producer fifo_file1 5 in2_s.txt 60 &

check_test1:
	./check_line in1_s.txt 2 out_NP_1K_s.txt
	./check_line in2_s.txt 5 out_NP_1K_s.txt
	./check_line in3_s.txt 10 out_NP_1K_s.txt

test2: Consumer Producer
	./Consumer fifo_file2 out_1P_NK_s.txt 60 &
	./Consumer fifo_file2 out_1P_NK_s.txt 60 &
	./Consumer fifo_file2 out_1P_NK_s.txt 60 &
	./Producer fifo_file2 2 in1_s.txt 60 &

check_test2:
	./check_line in1_s.txt 2 out_1P_NK_s.txt

test3: Consumer Producer
	./Consumer fifo_file3 out_NP_NK_s.txt 60 &
	./Consumer fifo_file3 out_NP_NK_s.txt 60 &
	./Consumer fifo_file3 out_NP_NK_s.txt 60 &
	./Producer fifo_file3 2 in1_s.txt 60 &
	./Producer fifo_file3 10 in3_s.txt 60 &
	./Producer fifo_file3 5 in2_s.txt 60 &

check_test3:
	./check_line in1_s.txt 2 out_NP_NK_s.txt
	./check_line in2_s.txt 5 out_NP_NK_s.txt
	./check_line in3_s.txt 10 out_NP_NK_s.txt

test4: Consumer Producer
	./Consumer fifo_file4 out_NP_1K_m.txt 600 &
	./Producer fifo_file4 2 in1_m.txt 600 &
	./Producer fifo_file4 10 in3_m.txt 600 &
	./Producer fifo_file4 5 in2_m.txt 600 &

check_test4:
	./check_line in1_m.txt 2 out_NP_1K_m.txt
	./check_line in2_m.txt 5 out_NP_1K_m.txt
	./check_line in3_m.txt 10 out_NP_1K_m.txt

test5: Consumer Producer
	./Consumer fifo_file5 out_1P_NK_m.txt 600 &
	./Consumer fifo_file5 out_1P_NK_m.txt 600 &
	./Consumer fifo_file5 out_1P_NK_m.txt 600 &
	./Producer fifo_file5 2 in1_m.txt 600 &

check_test5:
	./check_line in1_m.txt 2 out_1P_NK_m.txt

test6: Consumer Producer
	./Consumer fifo_file6 out_NP_NK_m.txt 600 &
	./Consumer fifo_file6 out_NP_NK_m.txt 600 &
	./Consumer fifo_file6 out_NP_NK_m.txt 600 &
	./Producer fifo_file6 2 in1_m.txt 600 &
	./Producer fifo_file6 10 in3_m.txt 600 &
	./Producer fifo_file6 5 in2_m.txt 600 &

check_test6:
	./check_line in1_m.txt 2 out_NP_NK_m.txt
	./check_line in2_m.txt 5 out_NP_NK_m.txt
	./check_line in3_m.txt 10 out_NP_NK_m.txt

test7: Consumer Producer
	./Consumer fifo_file7 out_NP_1K_b.txt 6000 &
	./Producer fifo_file7 2 in1_b.txt 6000 &
	./Producer fifo_file7 10 in3_b.txt 6000 &
	./Producer fifo_file7 5 in2_b.txt 6000 &

check_test7:
	./check_line in1_b.txt 2 out_NP_1K_b.txt
	./check_line in2_b.txt 5 out_NP_1K_b.txt
	./check_line in3_b.txt 10 out_NP_1K_b.txt

test8: Consumer Producer
	./Consumer fifo_file8 out_1P_NK_b.txt 6000 &
	./Consumer fifo_file8 out_1P_NK_b.txt 6000 &
	./Consumer fifo_file8 out_1P_NK_b.txt 6000 &
	./Producer fifo_file8 2 in1_b.txt 6000 &

check_test8:
	./check_line in1_b.txt 2 out_1P_NK_b.txt

test9: Consumer Producer
	./Consumer fifo_file9 out_NP_NK_b.txt 6000 &
	./Consumer fifo_file9 out_NP_NK_b.txt 6000 &
	./Consumer fifo_file9 out_NP_NK_b.txt 6000 &
	./Producer fifo_file9 2 in1_b.txt 6000 &
	./Producer fifo_file9 10 in3_b.txt 6000 &
	./Producer fifo_file9 5 in2_b.txt 6000 &

check_test9:
	./check_line in1_b.txt 2 out_NP_NK_b.txt
	./check_line in2_b.txt 5 out_NP_NK_b.txt
	./check_line in3_b.txt 10 out_NP_NK_b.txt

check_line:
	gcc -Wall check_line.c -o check_line

Consumer:
	gcc -Wall Consumer.c -o Consumer

testP: Producer
	./Producer fifo_file 1 in1.txt 5
	./Producer fifo_file 3 in3.txt 5
	./Producer fifo_file 2 in2.txt 5

Producer:
	gcc -Wall Producer.c -o Producer

create_fifo_files:
	mkfifo -m 666 fifo_file1
	mkfifo -m 666 fifo_file2
	mkfifo -m 666 fifo_file3
	mkfifo -m 666 fifo_file4
	mkfifo -m 666 fifo_file5
	mkfifo -m 666 fifo_file6
	mkfifo -m 666 fifo_file7
	mkfifo -m 666 fifo_file8
	mkfifo -m 666 fifo_file9

cleanTxt:
	rm -f fifo_file10 fifo_file9 fifo_file8 fifo_file7 fifo_file6 fifo_file5 fifo_file4 fifo_file3 fifo_file2 fifo_file1
	> out_1P_NK_b.txt
	> out_1P_NK_m.txt
	> out_1P_NK_s.txt
	> out_NP_1K_s.txt
	> out_NP_1K_m.txt
	> out_NP_1K_b.txt
	> out_NP_NK_s.txt
	> out_NP_NK_m.txt
	> out_NP_NK_b.txt
	> out_5P.txt
