
all: supplier cook restaurant

test: all
	./restaurant 5 5

supplier:
	gcc -Wall supplier.c -o supplier

cook:
	gcc -Wall cook.c -o cook

restaurant:
	gcc -Wall restaurant.c -o restaurant

clean:
	rm -f restaurant cook supplier