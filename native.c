#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// ts functionality
void* mmalloc(size_t size);
void throw_RangeError(const char* msg); // memory may be a bit optimized by inlining

void* fromCodePoint_stack(const uint32_t* numN, size_t len, size_t count);
void* fromCodePoint_heap(const uint32_t* numN, size_t len, size_t count);
void dblog(const char* msg, ...);
void wdblog(const wchar_t* msg, ...);
int fromCodePoint(const uint32_t* numN, size_t count, char* Str);
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif
// determine wether stack allocation can be supported by the compiler
// if not it will be forcely replaced by the heap allocation
#ifndef VLA
#if (                                                                   \
        defined(__clang__) ||                                           \
        defined(__GNUC__)  ||                                           \
        (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)      \
    )
#define VLA 1
#else
#define VLA 0
#endif
#endif

#define MIN_REALLOC_REQUEST_SIZE 32     // how big must be offset to request realloc
#define MIN_HEAP_ALLOC_REQUEST_SIZE 150 // how big must be string to use heap (it is count, size will be count*4)
EXPORT void* _cfromCodePoint(const uint32_t* numN, size_t count) {
    #if VLA
        if (count <= MIN_HEAP_ALLOC_REQUEST_SIZE)
            return fromCodePoint_stack(numN, count * 4 + 2, count);
    #endif
        return fromCodePoint_heap(numN, count * 4 + 2, count);
}
// members to test in perfomance.c
void* test_stack(const uint32_t* numN, size_t count) {
    return fromCodePoint_stack(numN, count * 4 + 2, count);
}
void* test_heap(const uint32_t* numN, size_t count) {
    return fromCodePoint_heap(numN, count * 4 + 2, count);
}
// private members


// throw range error (with printf features)
// uses throw_RangeError which defined in ts as function(message) { throw new RangeError(message); }
// the final message is stored into buffer with 65 characters
void RangeError(const char* msg, ...) {
    char buf[66];
    va_list args;
    va_start(args, msg);
    snprintf(buf, sizeof(buf), msg, args);
    va_end(args);
    throw_RangeError(msg);
}
// debug log with printf format when DEBUG defined
void dblog(const char* msg, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
#endif
}
// debug log with wprintf format, when DEBUG defined
void wdblog(const wchar_t* msg, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, msg);
    vwprintf(msg, args);
    va_end(args);
#endif
}
// does not exist when !VLA and therefore call will lead to compile-time err
// use more efficient stack pre-allocation than heap
#if VLA
void* fromCodePoint_stack(const uint32_t* numN, size_t len, size_t count) {

    dblog("stack_alloc: %zu\n", len);
    char stack[len];
    len -= fromCodePoint(numN, count, stack);
    char* heap = mmalloc(len);
    memcpy(heap, stack, len);
    // null terminate the string if required
    heap[len] = L'\0';
    wdblog(L"result: %ls\n", heap);
    return heap;

}
#endif
// use heap pre-allocation
// not so fast as stack allocation, but not slow in common execution practise
void* fromCodePoint_heap(const uint32_t* numN, size_t len, size_t count) {
    dblog("heap_alloc: %zu\n", len);
    char* heap = mmalloc(len);
    int offset = fromCodePoint(numN, count, heap);
    if (offset >= MIN_REALLOC_REQUEST_SIZE) {
        len -= offset;
        char* newstr = realloc(heap, len);
        if (newstr != NULL) heap = newstr;
    }
    heap[len] = L'\0';
    wdblog(L"result: %ls\n", heap);
    return heap;
}
int fromCodePoint(const uint32_t* numN, size_t count, char* Str) {
    // iterate through string and codePoint
    int offset = 0;
    dblog("fromCodePoint: numN: %p;count: %zu;Str:%p\n", numN, count, Str);
    // a good debug, should be removed out by compiler optimization when no debug
    int kind = 0; // 0 - bmp char
    int _count = 0;
    for (size_t i = 0; i < count; ++i) {
        uint32_t codePoint = numN[i];
        if (codePoint <= 0xFFFF) {
            if (kind == 0) {
                _count++;
            } else if (kind == 1) {
                dblog("Supplementary char[%d]\n", _count);
                _count = 0;
                kind = 0;
            } else {
                kind = 0;
                _count = 1;
            }
            // BMP character
            // it consumes 2 bytes
            // so we're adding to offset other 2 bytes
            offset += 2;
            *Str++ = (codePoint >> 8) & 0xFF; // High byte
            *Str++ = codePoint & 0xFF;        // Low byte

        } else if (codePoint <= 0x10FFFF) {
            if (kind == 1) {
                _count++;
            } else if (kind == 0) {
                dblog("BMP char[%d]\n", _count);
                _count = 0;
                kind = 1;
            } else {
                kind = 1;
                _count = 1;
            }
            // Supplementary character
            // it consumes 4 bytes
            codePoint -= 0x10000;
            uint16_t highSurrogate = (codePoint >> 10) + 0xD800;
            uint16_t lowSurrogate = (codePoint & 0x3FF) + 0xDC00;
            *Str++ = (highSurrogate >> 8) & 0xFF; // High surrogate high byte
            *Str++ = highSurrogate & 0xFF;        // High surrogate low byte
            *Str++ = (lowSurrogate >> 8) & 0xFF;  // Low surrogate high byte
            *Str++ = lowSurrogate & 0xFF;         // Low surrogate low byte
        } else {
            dblog("fromCodePoint throws RangeError at index %zu: \n", i);
            RangeError("Invalid code point %u", codePoint);
        }
    }
    if (_count != 0) {
        if (kind == 0)
            dblog("BMP");
        else
            dblog("Supplementary");
        dblog(" char[%d]\n", _count);
    }
    return offset;
}
/*
    It is used to prevent unused functions from being removed by the linker.
    Though this can be removed and done in the linker's command arguments
*/
void keepfn(void* p1, void* p2) {
    printf("%s: %d: keepfn is a keep function\n", __FILE__, __LINE__);
    exit(1);
    p1 = malloc(1);
    p2 = realloc(p1, 1);
    free(p2);
    memcpy(p1, p2, 0);
    wprintf(L"\n");
    printf("\n");
}