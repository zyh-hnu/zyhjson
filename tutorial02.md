tutorial02用于解析数字

##### 1 练习1

合并 `lept_parse_null()` 、 `lept_parse_true()` 和 `lept_parse_false()`函数

>因为违反DRY原则，即 don't repeat yourself()

修改test.c中的测试文件，例如将`test_parse_invalid_value()`编写为宏进行替换

容易维护，但是对于性能会带来影响。



##### 2 数字语法

```c++
number = [ "-" ] int [ frac ] [ exp ]	
int = "0" / digit1-9 *digit	//0或者1-9后跟任意数字闭包
frac = "." 1*digit	//小数点后是一或者多个数字
exp = ("e" / "E") ["-" / "+"] 1*digit
```

number 是以十进制表示，它主要由 4 部分顺序组成：负号、整数、小数、指数。只有整数是必需部分。注意和直觉可能不同的是，正号是不合法的。

整数部分如果是 0 开始，只能是单个 0；而由 1-9 开始的话，可以加任意数量的数字（0-9）。也就是说，`0123` 不是一个合法的 JSON 数字。

小数部分比较直观，就是小数点后是一或多个数字（0-9）。

JSON 可使用科学记数法，指数部分由大写 E 或小写 e 开始，然后可有正负号，之后是一或多个数字（0-9）。

>在老师leptjson中，选择双精度浮点数来存储json数字



##### 3 主要任务

1. 重构合并 `lept_parse_null()`、`lept_parse_false()`、`lept_parse_true()` 为 `lept_parse_literal()`。get！
2. 加入 [维基百科双精度浮点数](https://en.wikipedia.org/wiki/Double-precision_floating-point_format#Double-precision_examples) 的一些边界值至单元测试，如 min subnormal positive double、max double 等。
3. 去掉 `test_parse_invalid_value()` 和 `test_parse_root_not_singular()` 中的 `#if 0 ... #endif`，执行测试，证实测试失败。按 JSON number 的语法在 lept_parse_number() 校验，不符合标准的情况返回 `LEPT_PARSE_INVALID_VALUE` 和 `LEPT_PARSE_ROOT_NOT_SINGULAR` 错误码
4. 去掉 `test_parse_number_too_big` 中的 `#if 0 ... #endif`，执行测试，证实测试失败。仔细阅读 [`strtod()`](https://en.cppreference.com/w/c/string/byte/strtof)，看看怎样从返回值得知数值是否过大，以返回 `LEPT_PARSE_NUMBER_TOO_BIG` 错误码。（提示：这里需要 `#include` 额外两个标准库头文件。）

以上最重要的是第 3 条题目，就是要校验 JSON 的数字语法。建议可使用以下两个宏去简化一下代码：

~~~c
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
~~~

另一提示，在校验成功以后，我们不再使用 `end` 指针去检测 `strtod()` 的正确性，第二个参数可传入 `NULL`。

---

主要任务3中的需要通过的测试样例如下：

```c++
 TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "+0");
 TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "+1");
 TEST_ERROR(ZYH_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
 TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
 TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "INF");
 TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "inf");
 TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "NAN");
 TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "nan");

TEST_ERROR(ZYH_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
TEST_ERROR(ZYH_PARSE_ROOT_NOT_SINGULAR, "0x0");
TEST_ERROR(ZYH_PARSE_ROOT_NOT_SINGULAR, "0x123");
```

>练习二主要写一个检测JSON数字语法的函数

---

##### 4 strtod函数

函数的作用是将字符串中的数字部分转换为对应的浮点数，并将剩余的部分存储在 endptr 指向的地址中。
如果转换成功，则返回转换后的浮点数值；
如果转换失败（例如字符串不是有效的数字格式），则返回0.0。

```c++
	v->u.n = strtod(c->json, &end);	//字符串转为double
	if ((v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
		return ZYH_PARSE_NUMBER_TOO_BIG;
	c->json = p;	//end指向JSON待解析字符串
	v->type = ZYH_NUMBER;
```

