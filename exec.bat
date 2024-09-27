mkdir test
clang -c -O3 -std=c17 -DDEBUG -FORCE_UTF=8 native.c -o test/native.o
llvm-ar rcs test/native.lib test/native.o
tsc.exe --lib=test/native --emit=exe String.ts -o test/test.exe
cd test
test.exe
cd ..