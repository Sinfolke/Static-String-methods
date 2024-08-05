clang -c -O3 -std=c17 -DDEBUG native.c -o native.o
llvm-ar rcs native.lib native.o
tsc.exe --lib=native --emit=exe String.ts -o test.exe
test.exe 