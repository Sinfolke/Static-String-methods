type size_t = ulong;
declare type wchar_t = short;
class bad_alloc { // can be, or even must be replaced to InternalError
   public name: string = "bad_alloc";
   public message: string = "allocation error";
   public cause: string = "It occurs when no heap memory enough to allocate bytes. You should ensure you're allocating valid memory size and/or close unnecessary apps to free up RAM. Sometimes just restart the program.";
   public stack?: string;
   constructor(message?: string) {
     if (message) this.message = message;
   }
}
class RangeError {
    public name: string = "RangeError";
    public message: string;
    public cause?: string = "Occurs when a value is not in the set or range of allowed values";
    public stack?: string;
    constructor(message: string, cause?: string) {
        this.message = message;
        if (cause) this.cause = cause;
    }
}

declare function malloc(size: size_t): Opaque;
// declare function realloc(src: Opaque, newsize: size_t): Opaque;
// declare function free(mem: Opaque): void;
// declare function memcpy(dest: Opaque, src: Opaque, bytes: int): Opaque;
// declare function memmove(dest: Opaque, src: Opaque, bytes: int): Opaque;
declare function uprint(text: string): int; // prints depends whether choosen utf16 or utf8 encoding
declare function _cfromCharCode(numN: u16[], len: size_t): string;
declare function _cfromCodePoint(numN: double[], len: size_t): string;
declare function _cchangeLocale(): void;

function mmalloc(size: size_t): Opaque {
   // we add some under the hood check for null, but can be removed if not need
   let src: Opaque = malloc(size);
   if (src == null)
      throw new bad_alloc();
   return src;
}
static class String {
   fromCharCode(...numN: u16[]): string {
    if (numN.length == 0) return "";
    // both ts and C implementations work well
    let len: size_t = (numN.length + 1) * sizeof(wchar_t);
    let str: string = mmalloc(len);
    memcpy(str, numN, len);
    // null terminate the string
    for (let i: wchar_t = 0; i < sizeof(wchar_t); i++) {
      str[len + i] = '\0';
    }
    return str;
    return _cfromCharCode(numN, numN.length);
   }
   fromCodePoint(...numN: double[]): string {
    if (numN.length == 0) return "";
    return _cfromCodePoint(numN, numN.length);
   }
}

// function main() {
//    _cchangeLocale(); // change locale to output utf16 correctly
//    uprint(String.fromCodePoint(0x4F60));  // it uses wprintf to output utf16 characters
//    uprint(String.fromCharCode(0x4f60));
// }