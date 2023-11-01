# GxLib
[中文](README-zh.md)

A versatile cross-platform C++17 class library.

## Features
- [Allocator](gx/include/gx/allocator.h): Provides LinearAllocator, HeapAllocator, PoolAllocator, and a tool class to assist in using allocators: Pond.
- [GByteArray](gx/include/gx/gbytearray.h): Byte array class, providing operations such as read and write, HASH calculation, compression and decompression, base64 encoding and decoding for continuous binary data.
- [GCrypto](gx/include/gx/gcrypto.h): Provided some algorithms based on ECC encryption.
- [GFile](gx/include/gx/gfile.h): 
  1. Provide file operations: information acquisition, continuous read and write, random read and write, create, delete, rename; 
  2. Provide directory operations such as creating, deleting, renaming, and listing files.
- [GHashSum](gx/include/gx/ghash_sum.h): Provide generation functions for Md5, Sha1, and Sha256.
- [GIDAllocator](gx/include/gx/gid_allocator.h): ID fast allocation algorithm.
- [GThread](gx/include/gx/gthread.h): Thread class provides convenient methods for creating and using threads, as well as methods for setting thread names and thread priorities.
- [GTime](gx/include/gx/gtime.h): Providing time acquisition and operation for system clock and steady clock, providing mathematical operations for various time units, and supporting output in text format in the current time zone or UTC time.
- [GTimer](gx/include/gx/gtimer.h): Provide timers and their scheduling tools.
- [GUuid](gx/include/gx/guuid.h): Generate UUID and provide string output in multiple formats.
- [GVersion](gx/include/gx/gversion.h): Version number conversion and comparison tool.
- [Os](gx/include/gx/os.h): Provide dlOpen, dlSym native library loading and calling functions, provide program environment variable acquisition function, and provide system basic information acquisition function.
- [TaskSystem](gx/include/gx/task_system.h): Multi threaded task system (thread pool).

## Third party libraries used
- [zlib](https://github.com/madler/zlib)
- [tweetnacl](https://tweetnacl.cr.yp.to)
- [gtest](https://github.com/google/googletest)

## License
`GxLib` is licensed under the [MIT License](LICENSE.txt).
