BUILD = build
SOURCE = src

CC = gcc 
CFLAGS = -c -Wall -std=gnu99

all: chef cook1 cook2 cook3


chef: chef.o my_defines.h
	@echo " Compile chef ...";
	$(CC) -o chef chef.o -lpthread
ched.o: chef.c
	$(CC) $(CFLAGS) chef.c


cook1: cook.o my_defines.h
	@echo " Compile internal ...";
	$(CC) -o cook1 cook.o -lpthread
cook.o: cook.c
	$(CC) $(CFLAGS) cook.c


cook2: cook.o my_defines.h
	@echo " Compile internal ...";
	$(CC) -o cook2 cook.o -lpthread


cook3: cook.o my_defines.h
	@echo " Compile internal ...";
	$(CC) -o cook3 cook.o -lpthread


.phony: clean

clean:
	@echo " Cleaning . . ."
	rm -f *.o chef cook1 cook2 cook3
