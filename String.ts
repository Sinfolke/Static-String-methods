class bad_alloc {
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
// function to throw a RangeError within C
function throw_RangeError(msg: string): void {
    throw new RangeError(msg);
}
declare function malloc(size: u64): Opaque;
declare function realloc(src: Opaque, newsize: u64): Opaque;
declare function free(mem: Opaque): void;
declare function memcpy(dest: Opaque, src: Opaque, bytes: int): Opaque;
declare function memmove(dest: Opaque, src: Opaque, bytes: int): Opaque;
declare function wprintf(str: Opaque): void;
declare function printf(str: Opaque): void;
declare function _cfromCodePoint(numN: u32[], len: u64): string;
function mmalloc(size: u64): string {
   let src: string = malloc(size);
   if (src == null)
      throw new bad_alloc();
   return src;
}
static class String {
   fromCharCode(...numN: u16[]): string {
    if (numN.length == 0) return "";
    let size: u64 = numN.length * 2 + 1;
    let str: string = mmalloc(size);
    memcpy(str, numN, size);
    return str;
   }
   fromCodePoint(...numN: u32[]): string {
      // can be rewritten in ts if add vla feature and more advanced pointer management
    return _cfromCodePoint(numN, numN.length);
   }
}

// function main() {
//     // do not print characters to console correctly (i also tryed a bit adjust locale),
//     // but the convertion itself is done correctly
//     String.fromCodePoint(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
//     // this works correct except also do not print utf16 characters
//     let str: string = String.fromCharCode(1);

//     print("Hello, World!");
// }