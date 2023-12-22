// zyhjson.cpp: 定义应用程序的入口点。
//

#include "zyhjson.h"
#include <assert.h>

//定义宏 EXPECT(c,ch)，判断c指向的字符和ch是否相等
#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

//zyh_context，用来存放json字符串
typedef struct {
	const char* json;
}zyh_context;


/* null  = "null" */
static int zyh_parse_null(zyh_context* c, zyh_value* v) {
	EXPECT(c, 'n');	//断言，判断当前字符n
	if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
		return ZYH_PARSE_INVALID_VALUE;	//返回无效值
	c->json += 3;
	v->type = ZYH_NULL;
	return ZYH_PARSE_OK;
}

static int zyh_parse_false(zyh_context* c, zyh_value* v) {
	EXPECT(c, 'f');	//断言，判断当前字符f
	if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's'||c->json[3]!='e')
		return ZYH_PARSE_INVALID_VALUE;	//返回无效值
	c->json += 4;
	v->type = ZYH_FALSE;
	return ZYH_PARSE_OK;
}

static int zyh_parse_true(zyh_context* c, zyh_value* v) {
	EXPECT(c, 't');	//断言，判断当前字符n
	if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
		return ZYH_PARSE_INVALID_VALUE;	//返回无效值
	c->json += 3;
	v->type = ZYH_TRUE;
	return ZYH_PARSE_OK;
}

/* value = null / false / true */
/* 提示：下面代码没处理 false / true，将会是练习之一 */
static int zyh_parse_value(zyh_context* c, zyh_value* v) {
	switch (*c->json) {
	case 'n':  return zyh_parse_null(c, v);
	case 'f':	return zyh_parse_false(c, v);
	case 't':	return zyh_parse_true(c, v);
	case '\0': return ZYH_PARSE_EXPECT_VALUE;	//返回空白值
	default:   return ZYH_PARSE_INVALID_VALUE;	//返回无效值
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
/* 提示：这里应该是 JSON-text = ws value ws */
/* 以下实现没处理最后的 ws 和 LEPT_PARSE_ROOT_NOT_SINGULAR */
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
		if(*c.json!='\0')
			ret=ZYH_PARSE_ROOT_NOT_SINGULAR;
	}
	return ret;
}

//返回zyh_type枚举类型
zyh_type zyh_get_type(const zyh_value* v)//获取其类型
{
	assert(v != NULL);	//如果v不为空，则继续执行；否则出错
	return v->type;
}