all:
	-rm -rf test.o test.elf
	clang test.cpp -target x86_64-none-elf -Wall -Wextra -std=c++20 -o test.o
	#ld -nostdlib -static test.o -o test
