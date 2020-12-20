BUILD = build
SOURCE = src

CC = gcc 
CFLAGS = -c -Wall -std=gnu99

all: chef salad_maker

utils.o: utils.c
	$(CC) $(CFLAGS) utils.c

chef: chef.o utils.o my_defines.h
	@echo " Compile chef ...";
	$(CC) -o chef chef.o utils.o -lpthread
chef.o: chef.c
	$(CC) $(CFLAGS) chef.c


salad_maker: cook.o utils.o my_defines.h
	@echo " Compile internal ...";
	$(CC) -o salad_maker cook.o utils.o -lpthread
cook.o: cook.c
	$(CC) $(CFLAGS) cook.c


.phony: clean

clean:
	@echo " Cleaning . . ."
	rm -f *.o *.txt chef salad_maker
