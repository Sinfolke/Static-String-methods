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
// function String(val: any): string {
//    switch(typeof val) {
//       case "string":
//          return val;
//       case "number":
//          return _ctoString(val);
//       case "boolean":
//          return val ? "true" : "false";
//       case "undefined":
//          return "undefined";
//       case "object":
//          if (val === null) return "null";
//          let result: string = "";
//          if (Array.isArray(val)) {
//             const _a: any[] = <any[]>val;
//             for (let i: number = 0; i < _a.length - 1; ++i) {
//                result += String(_a[i]);
//                result += ", ";
//             }
//             result += _a[_a.length - 1];
//             result += "]";
//          } else {
//             result += "{";
//             for (const e in val) {
//                result += `${String(e)}: ${String(val[e])}`;
//                result += ",\n";
//             }
//             result += "}";
//          }
//          return result;
//       default:
//          // atlough it should be allowed to convert a function to a string.
//          return `<${typeof val}>`; // <Function> etc.
//    }
// }
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
    print(numN.length);
    return _cfromCodePoint(numN, numN.length);
   }
   // raw(callSite: { raw: string }, ...substitutions: any[]): string {
   //    let result = "";
   //    if (callSite.raw.length >= substitutions.length) {
   //       let i = 0;
   //       for (; i < substitutions.length; ++i) {
   //          result += callSite[i];
   //          result += String(substitutions[i]);
   //       }
   //       for (; i < callSite.raw.length; ++i) {
   //          result += callSite[i];
   //       }
   //    } else {
   //       for (let i = 0; i < callSite.raw.length; ++i) {
   //          result += callSite[i];
   //          result += String(substitutions[i]);
   //       }
   //    }
   //    return result;
   // }
   // String.raw`templateString
}

function main() {
   _cchangeLocale(); // change locale to output utf16 correctly
   let result = String.fromCodePoint(0x1F600, 0x2764, 0x2B50, 0x1F602, 0x1F916);
   print("len: " + result.length);
   uprint(result);  // it uses wprintf to output utf16 characters
   print("");
   //uprint(String.fromCharCode(0x4f60));
}