这是一个仿照leptjson完成的json解析库：[leptjson解析库github地址](https://github.com/miloyip/json-tutorial)  
项目环境：
- CMake: [下载地址](https://cmake.org/)
- visual studio 2022
---
其中文件中的老师的 JSON 库名为 leptjson，代码文件只有 3 个：
1. `leptjson.h`：leptjson 的头文件（header file），含有对外的类型和 API 函数声明。
2. `leptjson.c`：leptjson 的实现文件（implementation file），含有内部的类型声明和函数实现。此文件会编译成库。
3. `test.c`：我们使用测试驱动开发（test driven development, TDD）。此文件包含测试程序，需要链接 leptjson 库。
因此仿照编写了 zyhjson.h、zyhjson.c、test.c，具体功能类似。
