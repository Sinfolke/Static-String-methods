cd ..
clang -c -O3 -std=c17 -DDEBUG -FORCE_UTF=8 String/native.c -o native.o
llvm-ar rcs native.lib native.o
tsc.exe --lib=native --emit=exe String/String.ts -o String/test.exe
cd String
test.exe 