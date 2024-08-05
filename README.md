String.fromCharCode and fromCodePoint implementation
it primary relies onto C library 
# **String.ts**:
  **class bad_alloc**
      _throws when malloc returns null. I think can be replaced to js **InternalError**_

  **class RangeError**
      _a requirement for **fromCodePoint**. Not instance of **Error** class atlough it must be. It does not fully follow **js official doc** (in terms of constructor)_

   **function mmalloc**
       _uses **malloc** and throws a **bad_alloc** on **NULL**. Used both within **ts and C** for allocations_

   **Static class String**
# **native.c**:
   **_cfromCharCode**
        _A C implementation of the fromCharCode_
   **_cfromCodePoint**
        _used in **ts** code. It primary decides whether to use **heap** or **stack** preallocation_
   **_cchangeLocale**
        _To adjust the locale for the correct output_
   **uprint**
        _universal print - prints utf8 or utf16 sequence depends whether choosed utf16 or utf8 encoding_
   **isFinite**
     _A function needed in fromCodePoint for check_
   **isNaN**
     _A function needed in isFinite_

   **Private RangeError:**
        _throws **RangeError** with **printf** functionality and do exit(EXIT_FAILURE)_

   **Private dblog, wdblog:**
        _uses **printf**, **wprintf** if defined **DEBUG**_

   **Private fromCodePoint_stack:** 
        _function with **stack preallocation**_

   **Private fromCodePoint_heap:**
        _function with the **heap preallocation**_

   **Private fromCodePoint:**
        _internal byte manipulation function along with char_conv function_

   **Private keepfn:**
        _the function to keep exported other internal functions like **malloc**, **memcpy** and so on_
