// zyhjson.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#ifndef ZYHJSON_H__
#define ZYHJSON_H__

//声明zyh_type的枚举类型，第一个成员默认为0，后续加1
typedef enum{ ZYH_NULL, ZYH_FALSE, ZYH_TRUE, ZYH_NUMBER, ZYH_STRING, ZYH_ARRAY, ZYH_OBJECT}zyh_type;

//枚举类型，解析json得到的返回值
enum {
    ZYH_PARSE_OK = 0,   //解析成功
    ZYH_PARSE_EXPECT_VALUE,     //超出，只有空白
    ZYH_PARSE_INVALID_VALUE,    //无效值，解析不是三种字面值 null/ false/ true
    ZYH_PARSE_ROOT_NOT_SINGULAR //不唯一,一个值后，空白之后还有其他字符
}; 

//json是一个树形结构，需要解析为树形结构，因此设置树的节点为zyh_value
// 定义zyh_value结构体，其中包含zyh_type枚举类型的枚举变量type
typedef struct {
	zyh_type type;  //枚举变量 type
}zyh_value;

//parse函数解析json
/*
    lept_value v;   定义树节点结构体v
    const char json[] = ...;获取json字符串
    int ret = lept_parse(&v, json);解析json到节点v中，并返回值
*/
int zyh_parse(zyh_value* v, const char* json);

//获取解析成功后树中节点类型值
/*
    输入：已解析成功的节点v
    返回值：返回节点类型
*/
zyh_type zyh_get_type(const zyh_value* v);

#endif

// TODO: 在此处引用程序需要的其他标头。
