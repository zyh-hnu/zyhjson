// zyhjson.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#ifndef ZYHJSON_H__
#define ZYHJSON_H__

typedef enum{ ZYH_NULL, ZYH_FALSE, ZYH_TRUE, ZYH_NUMBER, ZYH_STRING, ZYH_ARRAY, ZYH_OBJECT}zyh_type;

enum {
    ZYH_PARSE_OK = 0,               //解析成功
    ZYH_PARSE_EXPECT_VALUE,         //超出，只有空白
    ZYH_PARSE_INVALID_VALUE,        //无效值，解析不是三种字面值 null/ false/ true
    ZYH_PARSE_ROOT_NOT_SINGULAR,    //不唯一,一个值后，空白之后还有其他字符
    ZYH_PARSE_NUMBER_TOO_BIG,       //数字过大
    ZYH_PARSE_MISS_QUOTATION_MARK,  //缺少引号，没有找到结束的双引号字符
    ZYH_PARSE_INVALID_STRING_ESCAPE,//无效的转义字符，如\x
    ZYH_PARSE_INVALID_STRING_CHAR   //无效的字符串字符，出现不合法字符，例如控制字符或无法识别的字符
}; 

typedef struct {
    union {
        struct { char* s; size_t len; }str;     /* string */
        double n;                               /* number */
    }u;
    zyh_type type;  //枚举变量 type
}zyh_value;

#define zyh_init(v) do { (v)->type = ZYH_NULL; } while(0)   /*初始化zyh_init*/

int zyh_parse(zyh_value* v, const char* json);

void zyh_free(zyh_value* v);

zyh_type zyh_get_type(const zyh_value* v);

#define zyh_set_null(v) zyh_free(v)

int zyh_get_boolean(const zyh_value* v);    //boolean
void zyh_set_boolean(zyh_value* v, int b);

double zyh_get_number(const zyh_value* v);  //number
void zyh_set_number(zyh_value* v, double n);

const char* zyh_get_string(const zyh_value* v);//string
size_t zyh_get_string_length(const zyh_value* v);
void zyh_set_string(zyh_value* v, const char* s, size_t len);

#endif
