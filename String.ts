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
declare function memcpy(dest: Opaque, src: Opaque, bytes: int): Opaque;
// declare function memmove(dest: Opaque, src: Opaque, bytes: int): Opaque;
declare function uprint(text: string): int; // prints depends whether choosen utf16 or utf8 encoding
declare function _cfromCharCode(numN: u16[], len: size_t): string;
declare function _cfromCodePoint(numN: double[], len: size_t): string;
declare function _cchangeLocale(): void;
declare function _chasFraction(d: double): boolean;
declare function _ctoString(d: double): string;

function mmalloc(size: size_t): Opaque {
   // we add some under the hood check for null, but can be removed if not need
   let src: Opaque = malloc(size);
   if (src == null)
      throw new bad_alloc();
   return src;
}
function String(val: any): string {
   if (typeof val === "string") return val;
   else if (typeof val === "number") return _ctoString(val);
   else if (typeof val === "boolean") return val ? "true" : "false";
   else if (typeof val === "undefined") return "undefined";
   else if (typeof val === "object") {
      if (val === null) return "null";
      let result: string = "";
      if (Array.isArray(val)) {
         const _a: any[] = val;
         for (let i: number = 0; i < _a.length - 1; ++i) {
            result += String(_a[i]);
            result += ", ";
         }
         result += _a[_a.length - 1];
         result += "]";
      } else {
         result += "{";
         for (const [key, value] in val) {
            result += `${String(key)}: ${String(value)}`;
            result += ",\n";
         }
         result += "}";
      }
      return result;
   } else {
      return `<${typeof val}>`;
   } 
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
   raw(callSite: string, ...substitutions: any[]): string {
      let result: string = "";
      if (callSite.length >= substitutions.length) {
         let i = 0;
         for (; i < substitutions.length; ++i) {
            result += callSite[i];
            result += String(substitutions[i]);
         }
         for (; i < callSite.length; ++i) {
            result += callSite[i];
         }
      } else {
         for (let i = 0; i < callSite.length; ++i) {
            result += callSite[i];
            result += String(substitutions[i]);
         }
      }
      return result;
   }
   // String.raw`templateString` should be implemented by the compiler
}

// function main() {
//    _cchangeLocale(); // change locale to output utf16 correctly
//    uprint(String.fromCodePoint(0x4F60));  // it uses wprintf to output utf16 characters
//    uprint(String.fromCharCode(0x4f60));
// }