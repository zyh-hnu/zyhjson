// zyhjson.cpp: 定义应用程序的入口点。
#include "zyhjson.h"
#include <assert.h>	
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <math.h>    /* HUGE_VAL */

#ifndef ZYH_PARSE_STACK_INIT_SIZE
#define ZYH_PARSE_STACK_INIT_SIZE 256
#endif

//定义宏 EXPECT(c,ch)，判断c指向的字符和ch是否相等
#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c,ch) do{*(char*)zyh_context_push(c,sizeof(char))=(ch);}while(0)	/*压栈操作*/

//zyh_context，用来存放json字符串
typedef struct {
	const char* json;
	char* stack;	//设置一个指向字符数组的指针stack
	size_t size;	//设置栈大小，size_t
	size_t top;		//设置栈顶，size_t
}zyh_context;

/*void* 可以指向任意类型的数据。
	用于第一次分配内存空间/超出大小的再分配问题
*/
static void* zyh_context_push(zyh_context* c, size_t size)
{
	void* ret;
	assert(size > 0);	//断言,size>0继续执行，否则中断
	if (c->top + size >= c->size) {
		if (c->size == 0)
		{
			c->stack = NULL;		//设置栈大小为NULL
			c->size = ZYH_PARSE_STACK_INIT_SIZE;	/*初始化栈大小*/
		}
		while (c->top + size >= c->size)
			c->size += c->size >> 1;  /* c->size * 1.5 */
		//c->stack = (char*)realloc(c->stack, c->size);
		void* new_stack = realloc(c->stack, c->size);
		if (new_stack == NULL) {
			// 处理内存分配失败的情况
			// 可以选择返回错误码或采取其他适当的处理方式
			// 这里简单地打印错误消息并返回 NULL
			printf("Memory allocation failed\n");
			return NULL;
		}
		c->stack = (char*)new_stack;
	}
	ret = c->stack + c->top;
	c->top += size;
	return ret;
}

static void* zyh_context_pop(zyh_context* c, size_t size) {
	assert(c->top >= size);
	return c->stack + (c->top -= size);
}

/*解析\u+XXXX后的XXXX存放到unsigned中*/
static const char* zyh_parse_hex4(const char* p, unsigned* u) {
	/* \TODO */
	int i = 0;
	*u = 0;
	for (i = 0; i < 4; i++) {
		char ch = *p++;
		*u <<= 4;
		if (ch >= '0' && ch <= '9')
			*u |= ch - '0';
		else if (ch >='A' && ch <= 'F')
			*u |= ch - ('A' - 10);
		else if (ch >='a' && ch <= 'f')
			*u |= ch - ('a' - 10);
		else
			return NULL;
	}
	return p;
}

/*将unsigned u按照字节编码*/
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

#define STRING_ERROR(ret) do{c->top=head;return ret;}while(0)

static int zyh_parse_string(zyh_context* c, zyh_value* v)
{
	size_t head = c->top, len;
	unsigned u,u2;
	const char* p;
	EXPECT(c, '\"');
	p = c->json;
	for (;;) {
		char ch = *p++;
		switch (ch) {
		case '\"':	//string "结束
			len = c->top - head;
			zyh_set_string(v, (const char*)zyh_context_pop(c, len), len);	//将string结果弹栈，赋值给v->u.str.s
			c->json = p;
			return ZYH_PARSE_OK;
		case '\\':	//转义字符处理
			switch (*p++) {
			case '\"': PUTC(c, '\"'); break;
			case '\\': PUTC(c, '\\'); break;
			case '/':  PUTC(c, '/'); break;
			case 'b':  PUTC(c, '\b'); break;
			case 'f':  PUTC(c, '\f'); break;
			case 'n':  PUTC(c, '\n'); break;
			case 'r':  PUTC(c, '\r'); break;
			case 't':  PUTC(c, '\t'); break;
			case 'u':
				if (!(p = zyh_parse_hex4(p, &u)))
					STRING_ERROR(ZYH_PARSE_INVALID_UNICODE_HEX);
				/*surrogate handing*/
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
				zyh_encode_utf8(c,u);
				break;
			default:
				STRING_ERROR(ZYH_PARSE_INVALID_STRING_ESCAPE);//无效的转义字符，如\x
			}
			break;
		case '\0':
			STRING_ERROR(ZYH_PARSE_MISS_QUOTATION_MARK);//缺少引号，没有找到结束的双引号字符
		default:
			if ((unsigned char)ch < 0x20) {	//0x20 (32)
				STRING_ERROR(ZYH_PARSE_INVALID_STRING_CHAR);
			}
			PUTC(c, ch);	//通常情况下压栈操作
		}
	}
}

static int zyh_parse_number(zyh_context* c, zyh_value* v) {
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
	errno = 0;
	v->u.n = strtod(c->json, NULL);	//字符串转为double
	if (errno == ERANGE &&(v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
		return ZYH_PARSE_NUMBER_TOO_BIG;
	c->json = p;	
	v->type = ZYH_NUMBER;
	return ZYH_PARSE_OK;
}

//实现null、true、false重构
static int zyh_parse_literal(zyh_context* c, zyh_value* v, const char* literal, zyh_type type)
{
	EXPECT(c, literal[0]);
	size_t size = strlen(literal);
	for (int i = 0; i < size - 1; i++)
	{
		if (c->json[i] != literal[i + 1])
			return ZYH_PARSE_INVALID_VALUE;
	}
	c->json += size;
	v->type = type;
	return ZYH_PARSE_OK;
}

/* value = null / false / true / number /string */
static int zyh_parse_value(zyh_context* c, zyh_value* v) {
	switch (*c->json) {
	case 'f':	return zyh_parse_literal (c, v, "false",ZYH_FALSE);
	case 'n':	return zyh_parse_literal(c, v,"null",ZYH_NULL);
	case 't':	return zyh_parse_literal(c, v,"true",ZYH_TRUE);
	default:   return zyh_parse_number(c, v);
	case '"':	return zyh_parse_string(c, v);
	case '\0': return ZYH_PARSE_EXPECT_VALUE;	//返回空白值
	}
}

/*跳过空白符 ws = *(%x20 / %x09 / %x0A / %x0D) */
static void zyh_parse_whitespace(zyh_context* c) {
	const char* p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	c->json = p;
}

void free_c_stack(zyh_context* c)
{
	if (c->stack != NULL)
	{
		free(c->stack);
	}
	c->stack = NULL;
}

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
	c.size = c.top = 0;
	c.stack = NULL;
	zyh_init(v);

	zyh_parse_whitespace(&c);	//跳过空格
	if ((ret = zyh_parse_value(&c, v)) == ZYH_PARSE_OK) {
		zyh_parse_whitespace(&c);	//跳过空格
		if (*c.json != '\0')
		{
			v->type = ZYH_NULL;
			ret = ZYH_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	assert(c.top == 0);
	free_c_stack(&c);
	return ret;
}

void zyh_free(zyh_value* v)
{
	assert(v != NULL);
	if (v->type == ZYH_STRING)
		free(v->u.str.s);
	v->type = ZYH_NULL;
}

//返回zyh_type枚举类型
zyh_type zyh_get_type(const zyh_value* v)//获取其类型
{
	assert(v != NULL);	//如果v不为空，则继续执行；否则出错
	return v->type;
}

/*boolean*/
int zyh_get_boolean(const zyh_value* v) {
	/* \TODO */
	assert(v != NULL && (v->type == ZYH_TRUE || v->type==ZYH_FALSE));
	return v->type == ZYH_TRUE;
}

void zyh_set_boolean(zyh_value* v, int b) {
	zyh_free(v);
	v->type = b ? ZYH_TRUE : ZYH_FALSE;
}

/*number*/
double zyh_get_number(const zyh_value* v)
{
	assert(v != NULL && v->type == ZYH_NUMBER);
	return v->u.n;
}

void zyh_set_number(zyh_value* v, double n) {
	zyh_free(v);
	v->u.n = n;
	v->type = ZYH_NUMBER;
}

/*string*/
const char* zyh_get_string(const zyh_value* v) {
	assert(v != NULL && v->type == ZYH_STRING);
	return v->u.str.s;
}

size_t zyh_get_string_length(const zyh_value* v)
{
	assert(v != NULL && v->type == ZYH_STRING);
	return v->u.str.len;
}

void zyh_set_string(zyh_value* v, const char* s, size_t len)
{
	assert(v != NULL && (s != NULL || len == 0));
	zyh_free(v);	//释放之前分配为string类型的内存
	v->u.str.s = (char*)malloc(len + 1);
	if (v->u.str.s != NULL) {	/*判断之前的内存分配是否成功*/
		if (s != NULL) {		/*判断传入的字符串是否为空*/
			memcpy(v->u.str.s, s, len);
		}
		v->u.str.s[len] = '\0';
		v->u.str.len = len;
		v->type = ZYH_STRING;
	}
}