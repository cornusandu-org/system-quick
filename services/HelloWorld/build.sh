set -e
g++ ./main.cpp -o binary
mv binary $1
nasm service.asm -o service.o -f elf64
ld service.o -shared -o service.so
mv service.so $2
