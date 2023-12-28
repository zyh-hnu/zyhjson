### tutorial04

>在第三单元中，已完成一般JSON字符串的解析，但是还存在转义序列 `\uXXXX`未被解析

##### 1 Unicode

ASCII，它是一种字符编码，把 128 个字符映射至整数 0 ~ 127。例如，`1` → 49，`A` → 65，`B` → 66 等等。这种 7-bit 字符编码系统非常简单，在计算机中以一个字节存储一个字符。适用于英语，但不适用于中日韩等文字

码点的范围是 0 至 0x10FFFF，码点又通常记作 U+XXXX，当中 XXXX 为 16 进位数字。例如 `劲` → U+52B2、`峰` → U+5CF0。

Unicode 还制定了各种储存码点的方式，这些方式称为  Unicode 转换格式（Uniform Transformation Format, UTF）。现时流行的 UTF 为 UTF-8、UTF-16 和 UTF-32。每种 UTF 会把一个码点储存为一至多个编码单元（code unit）。例如 UTF-8 的编码单元是 8 位的字节、UTF-16 为 16 位、UTF-32 为 32 位。除 UTF-32 外，UTF-8 和 UTF-16 都是可变长度编码。

---

##### 2 UFT-8编码

**码点code point：**将字符映射为一个整数码点，码点范围 `0~0x10FFFF`，码点又通常记作 U+XXXX，当中 XXXX 为 16 进位数字

**UTF8：**将码点存储为一个或者多个编码单元，UTF-8的编码单元是8位的字节

对于非转义字符，只要不少于32，可以直接复制结果

而对于 JSON字符串中的 `\uXXXX` 是以 16 进制表示码点 U+0000 至 U+FFFF，我们需要：

1. 解析 4 位十六进制整数为码点；
2. 由于字符串是以 UTF-8 存储，我们要把这个码点编码成 UTF-8。



U+0000 至 U+FFFF 这组 Unicode 字符称为基本多文种平面（basic multilingual plane, BMP），还有另外 16 个平面。那么 BMP 以外的字符，JSON 会使用代理对（surrogate pair）表示 `\uXXXX\uYYYY`。

高代理项码点范围：U+D800 至 U+DBFF

低代理项码点范围：U+DC00 至 U+DFFF

`codepoint = 0x10000 + (H − 0xD800) × 0x400 + (L − 0xDC00)`

**只有高代理项而缺乏低代理项：** 返回`LEPT_PARSE_INVALID_UNICODE_SURROGATE`错误

**有高代理，低代理不合法：**返回`LEPT_PARSE_INVALID_UNICODE_SURROGATE`错误

**\\u**：返回`LEPT_PARSE_INVALID_UNICODE_HEX` 错误



UTF-8 的编码单元为 8 位（1 字节），每个码点编码成 1 至 4 个字节。它的编码方式很简单，按照码点的范围，把码点的二进位分拆成 1 至最多 4 个字节：

|      码点范围      | 码点位数 |  字节1   |  字节2   |  字节3   |  字节4   |
| :----------------: | :------: | :------: | :------: | :------: | :------: |
|  U+0000 ~ U+007F   |    7     | 0xxxxxxx |          |          |          |
|  U+0080 ~ U+07FF   |    11    | 110xxxxx | 10xxxxxx |          |          |
|  U+0800 ~ U+FFFF   |    16    | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
| U+10000 ~ U+10FFFF |    21    | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |



我们举一个例子解析多字节的情况，欧元符号 `€` → U+20AC：

1. U+20AC 在 U+0800 ~ U+FFFF 的范围内，应编码成 3 个字节。
2. U+20AC 的二进位为 10000010101100
3. 3 个字节的情况我们要 16 位的码点，所以在前面补两个 0，成为 0010000010101100
4. 按上表把二进位分成 3 组：0010, 000010, 101100
5. 加上每个字节的前缀：11100010, 10000010, 10101100
6. 用十六进位表示即：0xE2, 0x82, 0xAC

对于这例子的范围，对应的 C 代码是这样的：

```c++
if (u >= 0x0800 && u <= 0xFFFF) {
    OutputByte(0xE0 | ((u >> 12) & 0xFF)); /* 0xE0 = 11100000 */
    OutputByte(0x80 | ((u >>  6) & 0x3F)); /* 0x80 = 10000000 */
    OutputByte(0x80 | ( u        & 0x3F)); /* 0x3F = 00111111 */
}
```

>需要处理代理对

##### 3 任务

1. 实现 `lept_parse_hex4()`，不合法的十六进位数返回 `LEPT_PARSE_INVALID_UNICODE_HEX`。
2. 按第 3 节谈到的 UTF-8 编码原理，实现 `lept_encode_utf8()`。这函数假设码点在正确范围 U+0000 ~ U+10FFFF（用断言检测）。
3. 加入对代理对的处理，不正确的代理对范围要返回 `LEPT_PARSE_INVALID_UNICODE_SURROGATE` 错误。

---

###### parse_hex4

`zyh_parse_hex4`函数作用：解析 4 位十六进制整数为unsigned u

> 这个函数的作用是将一个长度为 4 的十六进制字符串转换成 unsigned 类型的整数，并返回指向字符串中下一个字符的指针（即该字符串中的第5个字符）。该函数的输入参数 `p` 是指向要转换的十六进制字符串的指针，`u` 是指向输出结果的指针。

```c++
static const char* zyh_parse_hex4(const char* p, unsigned* u) {
	/* \TODO */
	int i = 0;
	*u = 0;
	for (i = 0; i < 4; i++) {
		char ch = *p++;
		*u <<= 4;
		if (ch >= '0' && ch <= '9')
			*u |= ch - '0';	//将ch字符转为整数，相当于将该数字添加到 u 的低四位中。
		else if (ch > 'A' && ch <= 'F')
			*u |= ch - ('A' - 10);
		else if (ch > 'a' && ch <= 'f')
			*u |= ch - ('a' - 10);
		else
			return NULL;
	}
	return p;
}
```

解析：其中`const char* p`指向待解析的字符串，如`\u1234`，而`unsigned* u`指向`unsigned`类型指针

为16位

`ch - '0'` 是将字符类型的数字转换为对应的整型数字

 `ch - ('A' - 10)` 是将字符类型的十六进制字母转换为对应的整型数字。在 ASCII 编码中，字母 'A' 的十进制值为 65，而常量 10 表示十六进制表示的数字与字符之间的偏移量。因此，`ch - ('A' - 10)` 的作用是将当前处理的十六进制字母转换为对应的整型数字，并将其与 `u` 原先的值进行按位或运算，相当于将该数字添加到 `u` 的低四位中。

例如：`\u123A`作为参数传入后，可得 0001 0010 0011 1010，计算得到十进制数为 4666，等于`0x123A`

---

###### encode_utf8

`zyh_encode_utf8`函数作用：将`unsigned u`无符号整数编码为字符

按照下表中的字节编码，一个字节8位，将unsigned编码。

|      码点范围      | 码点位数 |  字节1   |  字节2   |  字节3   |  字节4   |
| :----------------: | :------: | :------: | :------: | :------: | :------: |
|  U+0000 ~ U+007F   |    7     | 0xxxxxxx |          |          |          |
|  U+0080 ~ U+07FF   |    11    | 110xxxxx | 10xxxxxx |          |          |
|  U+0800 ~ U+FFFF   |    16    | 1110xxxx | 10xxxxxx | 10xxxxxx |          |
| U+10000 ~ U+10FFFF |    21    | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |

```c++
static void zyh_encode_utf8(zyh_context* c, unsigned u)
{
	if (u < 0x7F)
		PUTC(c, u & 0xFF);
	else if (u <= 0x7FF)
	{
		PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else if (u <= 0xFFFF)
	{
		PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else {
		PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
}
```

例如 `\u123A` 经过上个函数 `zyh_parse_hex4` 解析后得到 `unsigned u` 0001 0010 0011 1010

第一步：0xE0 1110 0000 | ((0001) & 1111 1111)=1110 0000| 0000 0001 = 1110 0001  0xE1

第二步：0x80 1000 0000| ((0001 0010 00) & 0011 1111)= 1000 0000| 0010 00= 1000 1000 0x88

第三步：0x80 1000 0000| ((0001 0010 0011 1010)& 0011 1111)=1000 0000| 0011 1010=1011 1010 0xBA

---

###### 举例说明：

我们举一个例子解析多字节的情况，欧元符号 `€` → U+20AC：

1. U+20AC 在 U+0800 ~ U+FFFF 的范围内，应编码成 3 个字节。
2. U+20AC 的二进位为 10000010101100 (`zyh_parse_hex4`函数作用)
3. 3 个字节的情况我们要 16 位的码点，所以在前面补两个 0，成为 0010000010101100
4. 按上表把二进位分成 3 组：0010, 000010, 101100
5. 加上每个字节的前缀：11100010, 10000010, 10101100（`zyh_encode_utf8`函数作用）
6. 用十六进位表示即：0xE2, 0x82, 0xAC

---

###### 高代理对和低代理对

```c++
if (u >= 0xD800 && u <= 0xDBFF)
{
    if (*p++!= '\\')
        STRING_ERROR(ZYH_PARSE_INVALID_UNICODE_SURROGATE);
    if (*p++!= 'u')
        STRING_ERROR(ZYH_PARSE_INVALID_UNICODE_SURROGATE);
    if (!(p = zyh_parse_hex4(p, &u2)))
        STRING_ERROR(ZYH_PARSE_INVALID_UNICODE_HEX);
    if (u2 < 0xDC00 || u2>0xDFFF)
        STRING_ERROR(ZYH_PARSE_INVALID_UNICODE_SURROGATE);
    u = 0x10000+((u - 0xD800)<<10)|(u2 - 0xDC00);
}
```

`codepoint = 0x10000 + (H − 0xD800) × 0x400 + (L − 0xDC00)`

**只有高代理项而缺乏低代理项：** 返回`LEPT_PARSE_INVALID_UNICODE_SURROGATE`错误

**有高代理，低代理不合法：**返回`LEPT_PARSE_INVALID_UNICODE_SURROGATE`错误

**\\u**：返回`LEPT_PARSE_INVALID_UNICODE_HEX` 错误

---

#####  4 总结

本单元完成了

- 对于JSON字符串解析中的`\uXXXX`转义字符的解析，通过本单元学习，了解了Unicode与UTF（Uniform Transformation Format, UTF）这种转换格式，UTF-8编码按照 **字节8位**进行编码，对于 `\uXXXX`的转义字符来说，需要对其4位的16进制转化为 `unsigned u`后，再对 `unsigned u` 进行编码并保存，例如 Dollar sign U+0024，进行处理需要将 16进制的0x0024转为 `unsigned u`后进行编码保存

- 但是对于 `\u+0000~\u+FFFF` 表示范围 < UCS中收录的字符集【统一字符集（Universal Coded Character Set, UCS） UCS 的码点是从 0 至 0x10FFFF 】，因此引入 **高低代理对**，高代理项码点范围：U+D800 至 U+DBFF，低代理项码点范围：U+DC00 至 U+DFFF。需要对高低代理对进行处理。

- **码点code point：**将字符映射为一个整数码点，码点范围 `0~0x10FFFF`，码点又通常记作 U+XXXX，当中 XXXX 为 16 进位数字

  **UTF8：**将码点存储为一个或者多个编码单元，UTF-8的编码单元是8位的字节

