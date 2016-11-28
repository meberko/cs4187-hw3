BINDIR = $(HOME)/bin
CC=gcc
CFLAGS=-c -Wall

build: hwcopy

hwcopy: hwcopy.o
	$(CC) -o $@ hwcopy.o
	@chmod 111 $@

test:
	@./test.sh

exec:
	@echo ""
	@echo "-------------------------------"
	@echo "running test with custom args"
	@echo "-------------------------------"
	hwcopy $(ARG)
	@echo ""

clean:
	@rm -rf *.o hwcopy
