# Martti Aukia 51657228
CC=gcc
CFLAGS=-Wall
SHELL:=/bin/bash

all: hexB
shell: filesys.o shell.o
shell.o: shell.c
	gcc -c shell.c -o shell.o
filesys.o: filesys.c filesys.h helpers.h
	gcc -c filesys.c -o filesys.o
clean:
	rm -f shell shell.o filesys.o
run: shell
	./shell
hexC: run
	hexdump -C virtualdiskC3_C1
hexD: run
	hexdump -C virtualdiskD3_D1
hexB: run
	hexdump -C virtualdiskB3_B1_a
diff: run
	# colordiff -u <(hexdump -C virtualdiskD3_D1) <(hexdump -C virtualdisk_test)
	colordiff -u <(hexdump -C virtualdiskD3_D1) <(hexdump -C virtualdiskC3_C1)
