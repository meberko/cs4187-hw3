BINDIR = $(HOME)/bin
CC=gcc
CFLAGS=-c -Wall

build: hwcopy

hwcopy: hwcopy.o
	$(CC) -o $@ hwcopy.o
	@chmod 111 $@

test:
	mkdir -p /tmp/hw3
	mkdir -p /tmp/dir
	@echo ""
	@echo "----------------------------------------------"
	@echo "|              	  TEST 1                      |"
	@echo "----------------------------------------------"
	@echo ""
	echo "ls ls" | ./hwcopy
	diff -s /bin/ls /tmp/ls
	@echo ""
	@echo "----------------------------------------------"
	@echo "|              	 TEST 2                      |"
	@echo "----------------------------------------------"
	@echo ""
	echo "cp /alias/cp" | ./hwcopy
	diff -s /bin/cp /tmp/hw3/cp
	@echo ""
	@echo "----------------------------------------------"
	@echo "|              	 TEST 3                      |"
	@echo "----------------------------------------------"
	@echo ""
	echo "cp /alias2/cp" | ./hwcopy
	diff -s /bin/cp /tmp/dir/cp
	@echo ""
	@echo "----------------------------------------------"
	@echo "|              	 TEST 4                      |"
	@echo "----------------------------------------------"
	@echo ""
	echo "../mkdir /mkdir" | ./hwcopy
	@echo ""
	@echo "----------------------------------------------"
	@echo "|              	 TEST 5                      |"
	@echo "----------------------------------------------"
	@echo ""
	echo "mkdir ../mkdir" | ./hwcopy
	@echo ""
	@echo "----------------------------------------------"
	@echo "|              	 TEST 6                      |"
	@echo "----------------------------------------------"
	@echo ""
	echo "mkdir /dir/../mkdir" | ./hwcopy
	diff -s /bin/mkdir /tmp/mkdir

clean:
	@rm -rf *.o hwcopy
