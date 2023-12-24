// zyhjson.cpp: 定义应用程序的入口点。
//

#include "zyhjson.h"
#include <assert.h>
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <math.h>    /* HUGE_VAL */

//定义宏 EXPECT(c,ch)，判断c指向的字符和ch是否相等
#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

//zyh_context，用来存放json字符串
typedef struct {
	const char* json;
}zyh_context;

//实现null、true、false重构
static int zyh_parse_literal(zyh_context* c, zyh_value* v,const char* literal,zyh_type type)
{
	EXPECT(c, literal[0]);
	size_t size = strlen(literal);
	for (int i = 0; i < size-1; i++)
	{
		if (c->json[i] != literal[i + 1])
			return ZYH_PARSE_INVALID_VALUE;
	}
	c->json += size;
	v->type = type;
	return ZYH_PARSE_OK;
}

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

static int zyh_parse_number(zyh_context* c, zyh_value* v) {
	char* end;
	/* \TODO validate number 添加验证数字格式的功能*/
	const char* p = c->json;
	if (*p == '-')p++;
	if (*p == '0')
	{
		p++;
		if (ISDIGIT(*p))return ZYH_PARSE_ROOT_NOT_SINGULAR;
		if(*p=='x')return ZYH_PARSE_ROOT_NOT_SINGULAR;
	}
	else{
		if (!ISDIGIT1TO9(*p)) return ZYH_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	if (*p == '.')
	{
		p++;
		if (!ISDIGIT(*p)) return ZYH_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-') p++;
		if (!ISDIGIT(*p)) return ZYH_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}

	/*函数的作用是将字符串中的数字部分转换为对应的浮点数，并将剩余的部分存储在 endptr 指向的地址中。
	如果转换成功，则返回转换后的浮点数值；
	如果转换失败（例如字符串不是有效的数字格式），则返回0.0。*/
	v->n = strtod(c->json, &end);	//字符串转为double
	if ((v->n == HUGE_VAL || v->n == -HUGE_VAL))
		return ZYH_PARSE_NUMBER_TOO_BIG;
	c->json = p;	//end指向JSON待解析字符串
	v->type = ZYH_NUMBER;
	return ZYH_PARSE_OK;
}

/* value = null / false / true / number*/
static int zyh_parse_value(zyh_context* c, zyh_value* v) {
	switch (*c->json) {
	case 'f':	return zyh_parse_literal (c, v, "false",ZYH_FALSE);
	case 'n':	return zyh_parse_literal(c, v,"null",ZYH_NULL);
	case 't':	return zyh_parse_literal(c, v,"true",ZYH_TRUE);
	default:   return zyh_parse_number(c, v);
	case '\0': return ZYH_PARSE_EXPECT_VALUE;	//返回空白值
	}
}


//作用：跳过空白字符ws
/* ws = *(%x20 / %x09 / %x0A / %x0D) */
static void zyh_parse_whitespace(zyh_context* c) {
	const char* p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	c->json = p;
}

//parse解析函数实现
int zyh_parse(zyh_value* v, const char* json)
{
	int ret;
	zyh_context c;
	/*
	*	v!=null为真继续执行
	*	v不为空继续执行，若为空则报错
	*/
	assert(v != NULL); 
	c.json = json;		
	v->type = ZYH_NULL;	//先写入为ZYH_NULL类型

	zyh_parse_whitespace(&c);	//跳过空格
	if ((ret = zyh_parse_value(&c, v)) == ZYH_PARSE_OK) {
		zyh_parse_whitespace(&c);	//跳过空格
		if (*c.json != '\0')
		{
			v->type = ZYH_NULL;
			ret = ZYH_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	return ret;
}

//返回zyh_type枚举类型
zyh_type zyh_get_type(const zyh_value* v)//获取其类型
{
	assert(v != NULL);	//如果v不为空，则继续执行；否则出错
	return v->type;
}

//返回zyh_value节点中的数值
double zyh_get_number(const zyh_value* v)
{
	assert(v != NULL && v->type == ZYH_NUMBER);
	return v->n;
}