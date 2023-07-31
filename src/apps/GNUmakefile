# This is a temporary Makefile

CXX_FLAGS=

all:
	clang ../libs/libc/premain.cpp -I../kernel/api -g -c -target x86_64-none-elf -std=c++20 -g -mno-red-zone -mno-80387 -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mcmodel=kernel -fno-stack-protector -fno-omit-frame-pointer -Wall -Wextra -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -ffreestanding -nostdlib -nodefaultlibs -o premain.o

	clang ../libs/libc/stdlib.cpp -I../kernel/api -g -c -target x86_64-none-elf -std=c++20 -g -mno-red-zone -mno-80387 -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mcmodel=kernel -fno-stack-protector -fno-omit-frame-pointer -Wall -Wextra -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -ffreestanding -nostdlib -nodefaultlibs -o stdlib.o

	nasm -felf64 ../libs/libc/crt0.asm -o crt0.o

	clang test.cpp -I../kernel/api -I../libs -g -c -target x86_64-none-elf -std=c++20 -g -mno-red-zone -mno-80387 -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mcmodel=kernel -fno-stack-protector -fno-omit-frame-pointer -Wall -Wextra -fno-rtti -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -ffreestanding -nostdlib -nodefaultlibs -o test.o
	ld -T linker.ld -nostdlib -static --no-dynamic-linker crt0.o premain.o stdlib.o test.o -o test
	mv test ../../base/

clean:
	rm *.o test
