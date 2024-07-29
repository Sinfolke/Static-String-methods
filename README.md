I decided to done those methods (fromCharsCode and fromCodePoint) so it could help you with String class. 
# **String.ts**:
  **class bad_alloc**
      _throws when malloc returns null. I think can be replaced to js **InternalError**_

  **class RangeError**
      _a requirement for **fromCodePoint**. Not instance of **Error** class atlough it must be. It does not follow js official doc, it is context defined_

   **function throw_RangeError**
      _used to throw a **RangeError** in **C**_

   **c function declarations...**

   **function mmalloc**
       _uses **malloc** and throws a **bad_alloc** on **NULL**. Used both within **ts and C** for allocations_

   **Static class String**
# **native.c**:

   **ts function declarations**

   **private function definitions on top**

   **preprocessor checks for EXPORT and VLA**
        _: **VLA** is used as stack optimization for low inputs. Can be adjusted by setting **-DVLA=0** or **-DVLA=1**_

   **constants** (_you'll get there_)

   **_cfromCodePoint**
        _used in **ts** code. It primary decides whether to use **heap** or **stack** preallocation_

   **test functions**
        _used in perfomance.c only_


   **Private RangeError:**
        _throws **RangeError** with **printf** functionality). I decided to limit the final message with stack **66** byte buffer_

   **Private dblog, wdblog:**
        _uses **printf**, **wprintf** if defined **DEBUG**_

   **Private fromCodePoint_stack:** 
        _function with **stack preallocation**_

   **Private fromCodePoint_heap:**
        _with the **heap preallocation**_

   **Private fromCodePoint:**
        _internal byte manipulation function (i checked, it follows String.fromCodePoint in **official js doc**)_

   **Private keepfn:**
        _the function to keep exported other internal functions like **malloc**, **memcpy** and so on_
