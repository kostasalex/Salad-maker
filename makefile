BUILD = build
SOURCE = src

CC = gcc 
CFLAGS = -c -Wall -std=gnu99

all: chef salad_maker

time_utils.o: time_utils.c
	$(CC) $(CFLAGS) time_utils.c

chef: chef.o time_utils.o my_defines.h
	@echo " Compile chef ...";
	$(CC) -o chef chef.o time_utils.o -lpthread
chef.o: chef.c
	$(CC) $(CFLAGS) chef.c


salad_maker: cook.o time_utils.o my_defines.h
	@echo " Compile internal ...";
	$(CC) -o salad_maker cook.o time_utils.o -lpthread
cook.o: cook.c
	$(CC) $(CFLAGS) cook.c


.phony: clean

clean:
	@echo " Cleaning . . ."
	rm -f *.o chef salad_maker
