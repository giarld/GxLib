# GxLib
一个通用的跨平台C++17类库。

## 功能
- [Allocator](gx/include/gx/allocator.h), 提供了 LinearAllocator、HeapAllocator、PoolAllocator 以及辅助使用分配器的工具类: Pond.
- [GByteArray](gx/include/gx/gbytearray.h): 字节数组类，为连续二进制数据提供读写、HASH计算、压缩解压缩、base64编码和解码等操作.
- [GCrypto](gx/include/gx/gcrypto.h): Provided some algorithms based on ECC encryption.
- [GFile](gx/include/gx/gfile.h):
    1. 提供文件操作：信息获取、连续读写、随机读写、创建、删除、重命名；
    2. 提供创建、删除、重命名和列出文件等目录操作。
- [GHashSum](gx/include/gx/ghash_sum.h): 为Md5、Sha1和Sha256提供生成功能。
- [GIDAllocator](gx/include/gx/gid_allocator.h): ID快速分配算法。
- [GThread](gx/include/gx/gthread.h): 线程类，提供了创建和使用线程的方便方法，以及设置线程名称和线程优先级的方法。
- [GTime](gx/include/gx/gtime.h): 提供系统时钟和稳定时钟的时间采集和操作，提供各种时间单位的数学运算，并支持在当前时区或UTC时间以文本格式输出。
- [GTimer](gx/include/gx/gtimer.h): 提供计时器及其调度工具。
- [GUuid](gx/include/gx/guuid.h): 生成UUID并提供多种格式的字符串输出。
- [GVersion](gx/include/gx/gversion.h): 版本号转换和比较工具。
- [Os](gx/include/gx/os.h): 提供dlOpen、dlSym原生库加载和调用功能，提供程序环境变量采集功能，提供系统基本信息采集功能。
- [TaskSystem](gx/include/gx/task_system.h): 多线程任务系统（线程池）。

## 使用的第三方库
- [zlib](https://github.com/madler/zlib)
- [tweetnacl](https://tweetnacl.cr.yp.to)
- [gtest](https://github.com/google/googletest)

## 许可
`GxLib` 根据 [MIT许可证](LICENSE.txt) 授权。
