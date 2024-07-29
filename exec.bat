clang -c -O3 -std=c17 native.c -o native.o
llvm-ar rcs native.lib native.o
tsc.exe --lib=native --emit=exe String2.ts -o test.exe
test.exe 