#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zyhjson.h"
#include "zyhjson.cpp"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

static void test_parse_null() {
    zyh_value v;
    zyh_init(&v);
    zyh_set_boolean(&v, 0); /*设置为ZYH_FALSE*/
    EXPECT_EQ_INT(ZYH_PARSE_OK, zyh_parse(&v, "null"));
    EXPECT_EQ_INT(ZYH_NULL, zyh_get_type(&v));
    zyh_free(&v);
}

static void test_parse_true() {
    zyh_value v;
    zyh_init(&v);
    zyh_set_boolean(&v, 0);
    EXPECT_EQ_INT(ZYH_PARSE_OK, zyh_parse(&v, "true"));
    EXPECT_EQ_INT(ZYH_TRUE, zyh_get_type(&v));
    zyh_free(&v);
}

static void test_parse_false() {
    zyh_value v;
    zyh_init(&v);
    zyh_set_boolean(&v, 1);
    EXPECT_EQ_INT(ZYH_PARSE_OK, zyh_parse(&v, "false"));
    EXPECT_EQ_INT(ZYH_FALSE, zyh_get_type(&v));
    zyh_free(&v);
}

#define TEST_NUMBER(expect, json)\
    do {\
        zyh_value v;\
        EXPECT_EQ_INT(ZYH_PARSE_OK, zyh_parse(&v, json));\
        EXPECT_EQ_INT(ZYH_NUMBER, zyh_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, zyh_get_number(&v));\
    } while(0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    /*当出现 "must underflow" 的情况时，计算机必须强制执行下溢，并将结果设置为 0。
    这是因为计算机不允许向下舍入到比它们的最小可能值更小的数字。*/
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    /*双精度浮点数测试样例*/
    TEST_NUMBER(0.0, "-0.00000000000000000000");
    TEST_NUMBER(0.00000000000000000001, "1e-20");
    TEST_NUMBER(0.00000000000000000009, "9e-20");
    TEST_NUMBER(0.00000000000000000010, "1e-19");
    TEST_NUMBER(0.999999999999999, "0.999999999999999");
    TEST_NUMBER(0.9999999999999999, "0.9999999999999999");
    TEST_NUMBER(1234567890123456.0, "1234567890123456");
    TEST_NUMBER(12345678901234560.0, "12345678901234560");
    TEST_NUMBER(-1234567890123456.0, "-1234567890123456");
    TEST_NUMBER(-12345678901234560.0, "-12345678901234560");
    TEST_NUMBER(1.234567890123456789, "1.234567890123456789");
    TEST_NUMBER(123456789012345678.9, "123456789012345678.9");
    TEST_NUMBER(1.234567890123456789e+30, "1.234567890123456789e+30");
    TEST_NUMBER(1.234567890123456789e-30, "1.234567890123456789e-30");
    TEST_NUMBER(1.234567890123456789e+300, "1.234567890123456789e+300");
    TEST_NUMBER(1.234567890123456789e-300, "1.234567890123456789e-300");
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
        zyh_value v;\
        zyh_init(&v);\
        EXPECT_EQ_INT(ZYH_PARSE_OK, zyh_parse(&v, json));\
        EXPECT_EQ_INT(ZYH_STRING, zyh_get_type(&v));\
        EXPECT_EQ_STRING(expect, zyh_get_string(&v), zyh_get_string_length(&v));\
        zyh_free(&v);\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
#if 0
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
#endif
}

#define TEST_ERROR(error, json)\
    do {\
        zyh_value v;\
        zyh_init(&v);\
        zyh_set_boolean(&v, 0);\
        EXPECT_EQ_INT(error, zyh_parse(&v, json));\
        EXPECT_EQ_INT(ZYH_NULL, zyh_get_type(&v));\
        zyh_free(&v);\
    } while(0)

static void test_parse_expect_value() {
    TEST_ERROR(ZYH_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(ZYH_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "?");
    /* invalid number */
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(ZYH_PARSE_INVALID_VALUE, "nan");

}

static void test_parse_root_not_singular() {
    TEST_ERROR(ZYH_PARSE_ROOT_NOT_SINGULAR, "null x");

    /* invalid number */
    TEST_ERROR(ZYH_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(ZYH_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(ZYH_PARSE_ROOT_NOT_SINGULAR, "0x123");

}

static void test_parse_number_too_big() {

    TEST_ERROR(ZYH_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(ZYH_PARSE_NUMBER_TOO_BIG, "-1e309");
}


static void test_parse_missing_quotation_mark() {
    TEST_ERROR(ZYH_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(ZYH_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {

    TEST_ERROR(ZYH_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(ZYH_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(ZYH_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(ZYH_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");

}

static void test_parse_invalid_string_char() {

    TEST_ERROR(ZYH_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(ZYH_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");

}

static void test_access_null() {
    zyh_value v;
    zyh_init(&v);
    zyh_set_string(&v, "a", 1);
    zyh_set_null(&v);
    EXPECT_EQ_INT(ZYH_NULL, zyh_get_type(&v));
    zyh_free(&v);
}

static void test_access_boolean() {
    zyh_value v;
    zyh_init(&v);
    zyh_set_string(&v, "a", 1);
    zyh_set_boolean(&v, 1);
    EXPECT_TRUE(zyh_get_boolean(&v));
    zyh_set_boolean(&v, 0);
    EXPECT_FALSE(zyh_get_boolean(&v));
    zyh_free(&v);
}

static void test_access_number() {
    zyh_value v;
    zyh_init(&v);
    zyh_set_string(&v, "a", 1);
    zyh_set_number(&v, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, zyh_get_number(&v));
    zyh_free(&v);
}

static void test_access_string() {
    zyh_value v;
    zyh_init(&v);
    zyh_set_string(&v, "", 0);
    EXPECT_EQ_STRING("", zyh_get_string(&v), zyh_get_string_length(&v));
    zyh_set_string(&v, "Hello", 5);
    EXPECT_EQ_STRING("Hello", zyh_get_string(&v), zyh_get_string_length(&v));
    zyh_free(&v);
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();

    test_parse_missing_quotation_mark(); //测试缺少最后引号   
    test_parse_invalid_string_escape(); //测试无效转义字符
    test_parse_invalid_string_char();   //测试无效字符

    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

int main() {
#ifdef _WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}