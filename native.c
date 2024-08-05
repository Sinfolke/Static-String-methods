
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>
#include <math.h>
#include <float.h>
void* mmalloc(size_t size);

void RangeError(const char* msg, ...);
void dblog(const char* msg, ...);
void wdblog(const wchar_t* msg, ...);
void keepfn(void* p1, void* p2);
#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

#if defined(_WIN32) && !defined(FORCE_UTF8)
    // we'd prefer to use utf16 encoding as it is standard for Windows
    #include <Windows.h>
    #define UTF16 1
    #define STDCHAR_T wchar_t
    #define STDCHAR_T_STR "wchar_t"
    #define _wsnprintf(buffer, size, format, ...) \
        swprintf(buffer, size, L##format, __VA_ARGS__)
    size_t char_conv(uint32_t codePoint, char* Str) {
        /*
            Converts codePoint to utf-16 encoding
            It is js standard but quite problematic.
        */

        // ref https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Global_Objects/String/fromCodePoint#:~:text=if%20(codePoint%20%3C%3D,lowSurrogate)%3B%0A%20%20%20%20%20%20%20%20%7D
        if (codePoint <= 0xFFFF) {
            // BMP character
            // it consumes 2 bytes
            // so we're adding to offset other 2 bytes
            *Str++ = (codePoint >> 8) & 0xFF; // High byte
            *Str++ = codePoint & 0xFF;        // Low byte
            return 2; // offset is 2
        } else { // codePoint <= 0x10FFFF
            // Supplementary character
            // it consumes 4 bytes
            codePoint -= 0x10000;
            uint16_t highSurrogate = (codePoint >> 10) + 0xD800;
            uint16_t lowSurrogate = (codePoint & 0x3FF) + 0xDC00;
            *Str++ = (highSurrogate >> 8) & 0xFF; // High surrogate high byte
            *Str++ = highSurrogate & 0xFF;        // High surrogate low byte
            *Str++ = (lowSurrogate >> 8) & 0xFF;  // Low surrogate high byte
            *Str++ = lowSurrogate & 0xFF;         // Low surrogate low byte
            return 0;  // zero offset
        }
    }
    EXPORT int uprint(const wchar_t* text) {
        // we use utf16 so print as wstring
        return printf("%ls", text);
    }
#else
    #define UTF16 0
    #define STDCHAR_T char
    #define STDCHAR_T_STR "char"
    #define _wsnprintf(buffer, size, format, ...) \
        snprintf(buffer, size, format, __VA_ARGS__)
    size_t char_conv(uint32_t codePoint, char* Str) {
        /*
            Converts the codePoint into utf-8 encoding
            It is not js standard, but may be less problematic compared to utf16,
            or at least better in usage
        */
        if (codePoint <= 0x7F) {
            // 1 byte
            *Str = (char) codePoint;
            return 3;
        } 
        else if (codePoint <= 0x7FF) {
            // 2 bytes
            *Str++ = (char) (0xC0 | (codePoint >> 6));
            *Str++ = (char) (0x80 | (codePoint & 0x3F));
            return 2;
        }
        else if (codePoint <= 0xFFFF) {
            // 3 bytes for BMP characters
            *Str++ = (char) (0xE0 | (codePoint >> 12));
            *Str++ = (char) (0x80 | ((codePoint >> 6) & 0x3F));
            *Str++ = (char) (0x80 | (codePoint & 0x3F));
            return 1;
        }
        else { // codePoint <= 0x10FFFF
            // 4 bytes for supplementary characters
            *Str++ = (char) (0xF0 | (codePoint >> 18));
            *Str++ = (char) (0x80 | ((codePoint >> 12) & 0x3F));
            *Str++ = (char) (0x80 | ((codePoint >> 6) & 0x3F));
            *Str++ = (char) (0x80 | (codePoint & 0x3F));
            return 0;
        }
    }
    EXPORT int uprint(const char* text) {
        // we use utf-8 so print as a regular string
        return printf("%s", text);
    }
#endif
// export symbols
EXPORT int isFinite(long double value);
EXPORT int isNaN(long double value);
EXPORT wchar_t* _cfromCharCode(uint16_t* numN, size_t count);
EXPORT void* _cfromCodePoint(const double* numN, size_t count);
EXPORT void _cchangeLocale();
EXPORT int _chasFraction(double d);
EXPORT STDCHAR_T* _ctoString(double d);

void* fromCodePoint_stack(const double* numN, size_t count, size_t len);
void* fromCodePoint_heap(const double* numN, size_t count, size_t len);
size_t fromCodePoint(const double* numN, size_t count, STDCHAR_T * Str);

// determine wether stack allocation can be supported by the compiler
// if not it will be forcely replaced by the heap allocation
#ifndef VLA
# if (                                                                    \
        defined(__clang__) ||                                             \
        defined(__GNUC__)  ||                                             \
        defined(_MSC_VER)  ||                                             \
        (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)        \
    )
#  define VLA 1               // we can use VLA
# else
#  define VLA 0               // we always rely on heap
# endif
#endif

#if VLA
# undef VLA
# ifdef _MSC_VER        
#  define VLA 2         // in MSVC we must use _alloca
# else
#  define VLA 1         // we can follow C99 standard (for better compatibility) and use char[size] consturct
# endif
#endif
#define MIN_REALLOC_REQUEST_SIZE 32      // how big must be offset to request realloc
#define MIN_HEAP_ALLOC_REQUEST_SIZE 1022 // how big must be string to use heap (it is count, size will be count*4 + 2)
                                         // here count is 1022 (max size 4096) - 4 kb
                                         // for linux max default stack size is 8 kb and for windows 1mb
// change locale to output chars correctly (both utf8 and utf16)
void _cchangeLocale() {
// this works on windows, i think on unix the setlocale would be enough
#ifdef WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    setlocale(LC_CTYPE, "en_US.UTF-8");
}
int isFinite(long double value) {
    return value < DBL_MAX && value > -DBL_MAX && !isnan(value);
}
int isNaN(long double value) {
    return isnan(value);
}
int _chasFraction(double d) {
    // Compute the fractional part by subtracting the integer part
    double fp = d - floor(d);
    // Return 1 (true) if the fractional part is greater than the defined tolerance
    return fabs(fp) >= 1e-10;
}
STDCHAR_T* _ctoString(double d) {
    // may be adjusted if the utf16 is taken as base encoding
    // 18,446,744,073,709,551,615 - 26 + 10 (for floating point) + 1 for null terminator = 37
    STDCHAR_T* str;
    STDCHAR_T buf[37];
    if (_chasFraction(d)) {
        // Significant fractional part
        int len = _wsnprintf(buf, sizeof(buf), "%.10f", d);
        // Remove trailing zeros
        STDCHAR_T *end = buf + len - 1;
        while (*end == '0') // end never >= buf
            end--;
        if (*end == '.')
            end--;
        len = (end - buf) * sizeof(STDCHAR_T);
        str = mmalloc(len);
        memcpy(str, buf, len + sizeof(STDCHAR_T)); // to copy with null terminator
    } else {
        // No significant fractional part
        int len = _wsnprintf(buf, sizeof(buf), "%.0f", d);
        str = mmalloc(len);
        memcpy(str, buf, len + sizeof(STDCHAR_T)); // to copy with null terminator
    }
    return str;
}
// fromCharCode C implementation
wchar_t* _cfromCharCode(uint16_t* numN, size_t count) {
    size_t len = (count + 1) * sizeof(wchar_t);
    wchar_t* str = mmalloc(len);
    memcpy(str, numN, len);
    // null terminate the string
    for (wchar_t i = 0; i < sizeof(wchar_t); i++) {
      str[len + i] = '\0';
    }
    return str;
}
void* _cfromCodePoint(const double* numN, size_t count) {
    dblog("%X\n", numN[0]);
    #if VLA
        if (count <= MIN_HEAP_ALLOC_REQUEST_SIZE)
            return fromCodePoint_stack(numN, count, count * 4 + sizeof(STDCHAR_T));
    #endif
        return fromCodePoint_heap(numN, count, count * 4 + sizeof(STDCHAR_T));
}

/*
        ### private members ###
*/


// does not exist when !VLA and therefore call will lead to compile-time err
// use more efficient stack pre-allocation than heap
#if VLA
void* fromCodePoint_stack(const double* numN, size_t count, size_t len) {
#if VLA == 1
    // gcc, clang or C99 implementation
    dblog("fromCodePoint: %s stack[%zu]\n", STDCHAR_T_STR, len);
    STDCHAR_T stack[len];
#else
    // MSVC implementation
    dblog("fromCodePoint: %s* stack = _alloca(%zu)\n", STDCHAR_T_STR, len);
    STDCHAR_T* stack = _alloca(len); // same as stack[len] construct, but in MSVC we are forced to call _alloca
#endif
    // manage the characters in 'stack' and remove the offset from length
    len -= fromCodePoint(numN, count, stack);
    STDCHAR_T* heap = mmalloc(len); // alloc heap memory with an exact size
    memcpy(heap, stack, len);       // copy
    // null terminate the string
    for (size_t i = 0; i < sizeof(STDCHAR_T); ++i)
        heap[len + i] = '\0';
    return heap;

}
#endif
// use heap pre-allocation
// not so fast as stack allocation, but not slow in common execution practise
void* fromCodePoint_heap(const double* numN, size_t count, size_t len) {
    dblog("fromCodePoint: %s* heap = mmalloc(%zu)\n", STDCHAR_T_STR, len);
    STDCHAR_T* heap = mmalloc(len); // pre-allocate max amount of bytes onto heap
    size_t offset = fromCodePoint(numN, count, heap); // manage characters and get offset
    len -= offset; // now len represents the actual string length
    if (offset >= MIN_REALLOC_REQUEST_SIZE) {
        // reduce the pre-allocated size if it is worth to
        STDCHAR_T* newstr = realloc(heap, len);
        if (newstr != NULL) heap = newstr;
    }
    // null terminate the string
    for (size_t i = 0; i < sizeof(STDCHAR_T); ++i)
        heap[len + i] = '\0';
    return heap;
}
size_t fromCodePoint(const double* numN, size_t count, STDCHAR_T* Str) {
    size_t offset = 0;
    dblog("fromCodePoint: numN: %p;count: %zu;Str:%p\n", numN, count, Str);
    for (size_t i = 0; i < count; ++i) {
        double codePoint = numN[i];

        if ( // ref https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Global_Objects/String/fromCodePoint#:~:text=if%20(%0A%20%20%20%20%20%20%20%20%20%20!isFinite(codePoint)%20%7C%7C%20//%20%60NaN%60%2C%20%60%2BInfinity%60%20%D0%B8%D0%BB%D0%B8%20%60%2DInfinity%60%0A%20%20%20%20%20%20%20%20%20%20codePoint%20%3C%200%20%7C%7C%20//%20%D0%BD%D0%B5%D0%B2%D0%B5%D1%80%D0%BD%D0%B0%D1%8F%20%D0%BA%D0%BE%D0%B4%D0%BE%D0%B2%D0%B0%D1%8F%20%D1%82%D0%BE%D1%87%D0%BA%D0%B0%20%D0%AE%D0%BD%D0%B8%D0%BA%D0%BE%D0%B4%D0%B0%0A%20%20%20%20%20%20%20%20%20%20codePoint%20%3E%200x10ffff%20%7C%7C%20//%20%D0%BD%D0%B5%D0%B2%D0%B5%D1%80%D0%BD%D0%B0%D1%8F%20%D0%BA%D0%BE%D0%B4%D0%BE%D0%B2%D0%B0%D1%8F%20%D1%82%D0%BE%D1%87%D0%BA%D0%B0%20%D0%AE%D0%BD%D0%B8%D0%BA%D0%BE%D0%B4%D0%B0%0A%20%20%20%20%20%20%20%20%20%20floor(codePoint)%20!%3D%20codePoint%20//%20%D0%BD%D0%B5%20%D1%86%D0%B5%D0%BB%D0%BE%D0%B5%20%D1%87%D0%B8%D1%81%D0%BB%D0%BE%0A%20%20%20%20%20%20%20%20)
            !isFinite(codePoint)            ||
            codePoint < 0                   ||
            codePoint > 0x10ffff            ||
            floor(codePoint) != codePoint   
        ) {
            dblog("fromCodePoint throws RangeError at index %zu: \n", i);
            // natively javascript would include floating point only when it does exist
            // so we'd use the same approach i think

            // ref https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Global_Objects/String/fromCodePoint#:~:text=throw%20RangeError(%22Invalid%20code%20point%3A%20%22%20%2B%20codePoint)%3B
            if (_chasFraction(codePoint))
                RangeError("Invalid code point %f", codePoint);
            else
                RangeError("Invalid code point %lld", (long long) codePoint);
        }
        // apply convertion for char according with encoding and get offset
        offset += char_conv((int32_t) codePoint, (char*) Str);


        // check below is being skipped since we do not need to worry for the array size such way
        // instead we allocate onto heap when the sequence is too large and sure the heap will never overflow


        // ref https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Global_Objects/String/fromCodePoint#:~:text=if%20(index%20%2B%201%20%3D%3D%20length%20%7C%7C%20codeUnits.length%20%3E%20MAX_SIZE)%20%7B%0A%20%20%20%20%20%20%20%20%20%20result%20%2B%3D%20stringFromCharCode.apply(null%2C%20codeUnits)%3B%0A%20%20%20%20%20%20%20%20%20%20codeUnits.length%20%3D%200%3B%0A%20%20%20%20%20%20%20%20%7D
        // if (i + 1 == count || i > 0x4000) {
        //   result += stringFromCharCode.apply(null, codeUnits);
        //   codeUnits.length = 0;
        // }
    }
    return offset;
}
// throw range error (with printf features)
// Throws an exception message to the console and calls exit()
// you may adjust if the throw function works differently
void RangeError(const char* msg, ...) {
    va_list args;
    va_start(args, msg);

    fprintf(stderr, "Range Error: ");
    vfprintf(stderr, msg, args);
    fputc('\n', stderr);

    va_end(args);
    exit(EXIT_FAILURE);
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