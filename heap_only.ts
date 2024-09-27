/*
    These functions can be done within the typescript without binding to a C library
    This will have following limitations:
     no stack optimizations. Actually not critical, but why not?
     always fixed character size, no configuration using preprocessor. Here i use wchar_t


*/
type size_t = ulong;
type double = number; // for a checker
type u32 = number; // for a checker
declare type Opaque = 0; // for a checker
declare function mmalloc(size: size_t): Opaque;
declare function isFinite(value: double): boolean;
declare function isNaN(value: double): boolean;
declare function floor(value: double): double;
function hasFraction(d: double): boolean {
    return floor(d) != d;
};
let MIN_REALLOC_REQUEST_SIZE = 500;
function char_conv(codePoint: u32, Str: Opaque, size: number): [number, number] {
    /*
        Converts codePoint to utf-16 encoding
        It is js standard but quite problematic.
    */

    // ref https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Global_Objects/String/fromCodePoint#:~:text=if%20(codePoint%20%3C%3D,lowSurrogate)%3B%0A%20%20%20%20%20%20%20%20%7D
    if (codePoint <= 0xFFFF) {
        // BMP character
        // it consumes 2 bytes
        // so we're adding to offset other 2 bytes
        Str[size++] = (codePoint >> 8) & 0xFF; // High byte
        Str[size++] = codePoint & 0xFF;        // Low byte
        return [2, size]; // offset is 2
    } else { // codePoint <= 0x10FFFF
        // Supplementary character
        // it consumes 4 bytes
        codePoint -= 0x10000;
        let highSurrogate: u16 = (codePoint >> 10) + 0xD800;
        let lowSurrogate: u16 = (codePoint & 0x3FF) + 0xDC00;
        Str[size++] = (highSurrogate >> 8) & 0xFF; // High surrogate high byte
        Str[size++] = highSurrogate & 0xFF;        // High surrogate low byte
        Str[size++] = (lowSurrogate >> 8) & 0xFF;  // Low surrogate high byte
        Str[size++] = lowSurrogate & 0xFF;         // Low surrogate low byte
        return [0, size];  // zero offset
    }
}
function _ctoString(d: double) {
    let buf: Opaque = mmalloc(40 + sizeof(wchar_t));
    let len;
    let offset = 0;
    if (hasFraction(d)) {
        // Significant fractional part
        len = swprintf(buf, 40, "%.9f", d);
        // Remove trailing zeros
        while(buf[len - offset] == '0' && len - offset > buf) {
            offset++;
        }
        len -= offset;
        buf[len] = '\0';
    } else {
        // No significant fractional part
        len = snprintf(buf, sizeof(buf), "%lld", d);
        offset = sizeof(buf) - len;
    }
    if (offset >= MIN_REALLOC_REQUEST_SIZE) {
        let new_heap = realloc(buf, len - offset);
        if (new_heap)
            buf = new_heap;
    }
    buf[len] = '\0';
    return buf;
}
function fromCodePoint(numN: double[], count: size_t, len: size_t) {
    let heap = mmalloc(len); // pre-allocate max amount of bytes onto heap
    let offset: size_t = 0;
    for (let i = 0; i < count; ++i) {
        let codePoint: double = numN[i];

        if ( // ref https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Global_Objects/String/fromCodePoint#:~:text=if%20(%0A%20%20%20%20%20%20%20%20%20%20!isFinite(codePoint)%20%7C%7C%20//%20%60NaN%60%2C%20%60%2BInfinity%60%20%D0%B8%D0%BB%D0%B8%20%60%2DInfinity%60%0A%20%20%20%20%20%20%20%20%20%20codePoint%20%3C%200%20%7C%7C%20//%20%D0%BD%D0%B5%D0%B2%D0%B5%D1%80%D0%BD%D0%B0%D1%8F%20%D0%BA%D0%BE%D0%B4%D0%BE%D0%B2%D0%B0%D1%8F%20%D1%82%D0%BE%D1%87%D0%BA%D0%B0%20%D0%AE%D0%BD%D0%B8%D0%BA%D0%BE%D0%B4%D0%B0%0A%20%20%20%20%20%20%20%20%20%20codePoint%20%3E%200x10ffff%20%7C%7C%20//%20%D0%BD%D0%B5%D0%B2%D0%B5%D1%80%D0%BD%D0%B0%D1%8F%20%D0%BA%D0%BE%D0%B4%D0%BE%D0%B2%D0%B0%D1%8F%20%D1%82%D0%BE%D1%87%D0%BA%D0%B0%20%D0%AE%D0%BD%D0%B8%D0%BA%D0%BE%D0%B4%D0%B0%0A%20%20%20%20%20%20%20%20%20%20floor(codePoint)%20!%3D%20codePoint%20//%20%D0%BD%D0%B5%20%D1%86%D0%B5%D0%BB%D0%BE%D0%B5%20%D1%87%D0%B8%D1%81%D0%BB%D0%BE%0A%20%20%20%20%20%20%20%20)
            !isFinite(codePoint)            ||
            codePoint < 0                   ||
            codePoint > 0x10ffff            ||
            hasFraction(codePoint) // floor(codePoint) != codePoint
        ) {
            // ref https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Global_Objects/String/fromCodePoint#:~:text=throw%20RangeError(%22Invalid%20code%20point%3A%20%22%20%2B%20codePoint)%3B
            RangeError("Invalid code point " + codePoint);
        }
        // apply convertion for char according with encoding and get offset
        offset += char_conv(codePoint, heap, i);


        // check below is being skipped since we do not need to worry for the array size such way
        // instead we allocate onto heap when the sequence is too large and sure the heap will never overflow


        // ref https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Global_Objects/String/fromCodePoint#:~:text=if%20(index%20%2B%201%20%3D%3D%20length%20%7C%7C%20codeUnits.length%20%3E%20MAX_SIZE)%20%7B%0A%20%20%20%20%20%20%20%20%20%20result%20%2B%3D%20stringFromCharCode.apply(null%2C%20codeUnits)%3B%0A%20%20%20%20%20%20%20%20%20%20codeUnits.length%20%3D%200%3B%0A%20%20%20%20%20%20%20%20%7D
        // if (i + 1 == count || i > 0x4000) {
        //   result += stringFromCharCode.apply(null, codeUnits);
        //   codeUnits.length = 0;
        // }
    }
    len -= offset; // now len represents the actual string length
    if (offset >= MIN_REALLOC_REQUEST_SIZE) {
        // reduce the pre-allocated size if it is worth to
        let new_heap = realloc(heap, len);
        if (new_heap)
            heap = new_heap;
    }
    // null terminate the string
    heap[len] = '\0';
    return heap;
}