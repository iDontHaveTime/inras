# inras

`inras` is an x86 assembler library made in C++20.

## Overview

Supported modes:
 - AMD64 (64bit mode)
 - x86 (32bit mode)
 - m16 (16bit mode, partial support)

## Roadmap

**1. Assembler frontend**
 - Simple assembling for AT&T syntax assembly.

**2. x86 Extensions**
 - SSE, SSE2, AVX2, x87, etc..

**3. Streaming API**

Left shift operator streaming, such as:
```c++
as::AssemblerStream asms;
asms << "movq $42, %rax";
```
Although final idea is not yet decided.

**4. File formats**

Support for ELF file emission, although as a separate library like `inrasELF` instead of being included in the core `inras` library.


## License

This project is licensed under the [**Boost Software License (BSL-1.0)**](LICENSE).