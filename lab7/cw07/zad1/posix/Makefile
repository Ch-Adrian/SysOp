
all: supplier cook restaurant

test: all
	./restaurant 5 5

supplier:
	gcc -Wall supplier.c -o supplier -lrt -lpthread

cook:
	gcc -Wall cook.c -o cook -lrt -lpthread

restaurant:
	gcc -Wall restaurant.c -o restaurant -lrt -lpthread

clean:
	rm -f restaurant cook supplier