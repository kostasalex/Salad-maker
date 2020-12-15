BUILD = build
SOURCE = src

CC = gcc 
CFLAGS = -c -Wall -std=gnu99

all: chef salad_maker


chef: chef.o my_defines.h
	@echo " Compile chef ...";
	$(CC) -o chef chef.o -lpthread
ched.o: chef.c
	$(CC) $(CFLAGS) chef.c


salad_maker: cook.o my_defines.h
	@echo " Compile internal ...";
	$(CC) -o salad_maker cook.o -lpthread
cook.o: cook.c
	$(CC) $(CFLAGS) cook.c



.phony: clean

clean:
	@echo " Cleaning . . ."
	rm -f *.o chef salad_maker
