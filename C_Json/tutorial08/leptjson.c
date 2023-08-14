#include "leptjson.h"
#include <assert.h>  // assert()
#include <errno.h> // errno, ERANGE
#include <math.h>  // HUGE_VAL
#include <stdio.h>   /* sprintf() */
#include <stdlib.h>  // NULL strtod() malloc(), realloc(), free()
#include <string.h> //memcpy()


#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#ifndef LEPT_PARSE_STRINGIFY_INIT_SIZE
#define LEPT_PARSE_STRINGIFY_INIT_SIZE 256
#endif

#define EXPECT(c, ch) do{assert(*c->json==(ch));c->json++;}while(0)
#define ISDIGIT(ch) ((ch)>='0'&&(ch)<='9')
#define ISDIGIT1TO9(ch) ((ch)>='1'&&(ch)<='9')
#define PUTC(c,ch) do{*(char*)lept_context_push(c,sizeof(char))=(ch);}while(0)
/* 使用宏 puts 去输出字符串 */
#define PUTS(c, s, len)     memcpy(lept_context_push(c, len), s, len)

/* 无论如何，在编程时都要考虑清楚变量的生命周期，特别是指针的生命周期。 */
/* 压入任意大小的数据 */
typedef struct {
	const char* json;
	char* stack;
	size_t size, top;
}lept_context;

/* 无论如何，在编程时都要考虑清楚变量的生命周期，特别是指针的生命周期。 */
/* 压入任意大小的数据 */
static void* lept_context_push(lept_context* c, size_t size) {
	void* ret;
	assert(size > 0);
	if (c->top + size >= c->size) {
		if (c->size == 0) {
			c->size = LEPT_PARSE_STACK_INIT_SIZE;
		}
		while (c->top + size >= c->size) {
			c->size += c->size >> 1;  
		}
		c->stack = (char*)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;  
	c->top += size;
	return ret;
}

/* 弹出任意大小的数据 */
static void* lept_context_pop(lept_context* c, size_t size) {
	assert(c->top >= size);
    // 返回栈顶的位置
	return c->stack + (c->top -= size);
}

/* 处理空白 */
static void lept_parse_whitespace(lept_context* c) {   
	const char* p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
		p++;
	}
	c->json = p; 
}

/* 合并 false、true、null 的解析函数 */
static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
	size_t i;
	EXPECT(c, literal[0]);
	for (i = 0; literal[i + 1]; i++) {
		if (c->json[i] != literal[i + 1]) {
			return LEPT_PARSE_INVALID_VALUE;
		}
	}
	c->json += i;
	v->type = type;
	return LEPT_PARSE_OK;
}

/* 将十进制的数字转换为二进制的 double，使用标准库的 strtod() 函数来进行转换。 */
static int lept_parse_number(lept_context* c, lept_value* v) {
	const char* p = c->json;
	if (*p == '-')p++;
	if (*p == '0') {
		p++;
	}
	else {
		if (!ISDIGIT1TO9(*p)) {
			return LEPT_PARSE_INVALID_VALUE;  
		}
		for (p++; ISDIGIT(*p); p++);  
	}
	if (*p == '.') {
		p++;
		if (!ISDIGIT(*p)) {
			return LEPT_PARSE_INVALID_VALUE;  
		}
		for (p++; ISDIGIT(*p); p++);   
	}
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-') {
			p++;
		}
		if (!ISDIGIT(*p))return LEPT_PARSE_INVALID_VALUE;  
		for (p++; ISDIGIT(*p); p++);
	}
	errno = 0;
	v->u.n = strtod(c->json, NULL);
	if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	v->type = LEPT_NUMBER;
	c->json = p;
	return LEPT_PARSE_OK;
}

/* 读 4 位 16 进制数字 */
static const char* lept_parse_hex4(const char* p, unsigned* u) {
	int i;
	*u = 0;
	for (i = 0; i < 4; i++) {
		char ch = *p++;
		*u <<= 4;
		if (ch >= '0' && ch <= '9')  *u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
		else return NULL;
	}
	return p;
}

/* 把码点编码成 utf-8 */
static void lept_encode_utf8(lept_context* c, unsigned u) {
	if (u <= 0x7F)
		PUTC(c, u & 0xFF);
	else if (u <= 0x7FF) {
		PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else if (u <= 0xFFFF) {
		PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else {
		assert(u <= 0x10FFFF);
		PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
}

/* 把返回错误码的处理抽取为宏，字符串错误宏 */
#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

/* 解析 JSON 字符串，把结果写入 str 和 len */
/* str 指向 c->stack 中的元素，需要在 c->stack */
static int lept_parse_string_raw(lept_context* c, char** str, size_t* len) {
	size_t head = c->top;
	unsigned u, u2;  
	const char* p;
	EXPECT(c, '\"');
	p = c->json;
	for (;;) {
		char ch = *p++;
		switch (ch) {
		case '\"':
			*len = c->top - head;
			*str =(char*) lept_context_pop(c, *len);
			c->json = p;
			return LEPT_PARSE_OK;
		case '\\':
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
					if (!(p = lept_parse_hex4(p, &u)))
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
					if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
						if (*p++ != '\\')
							STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
						if (*p++ != 'u')
							STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
						if (!(p = lept_parse_hex4(p, &u2)))
							STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
						if (u2 < 0xDC00 || u2 > 0xDFFF)
							STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
						u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
					}
					lept_encode_utf8(c, u);
					break;
				default:
					STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
			}
			break;
		case '\0':
			STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
		default:
			if ((unsigned char)ch < 0x20) {
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
			}
			PUTC(c, ch);
		}
	}
}

/* 将之前解析字符串的函数拆分为两部分，是为了在解析 json 对象的 key 值时，不使用 lept_value 存储键，因为这样会浪费其中的 type 这个无用字段 */
static int lept_parse_string(lept_context* c, lept_value* v) {
	int ret;
	char* s;
	size_t len;
	if ((ret = lept_parse_string_raw(c, &s, &len)) == LEPT_PARSE_OK) {
		lept_set_string(v, s, len);
	}
	return ret;
}

/* 前向声明：lept_parse_value() 会调用 lept_parse_array()，而 lept_parse_array() 又会调用 lept_parse_value()，这是互相引用，所以必须要加入函数前向声明。 */
static int lept_parse_value(lept_context* c, lept_value* v);

/* 解析数组 */
static int lept_parse_array(lept_context* c, lept_value* v) {
	size_t i, size = 0;
	int ret;
	EXPECT(c, '[');
	lept_parse_whitespace(c);
	if (*c->json == ']') {
		c->json++;
		v->type = LEPT_ARRAY;
		v->u.a.size = 0;
		v->u.a.e = NULL;
		return LEPT_PARSE_OK;  
	}
	for (;;) {
		lept_value e;
		lept_init(&e);
		if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK) {
			break;
		}
		memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));  
		size++;
		lept_parse_whitespace(c);
		if (*c->json == ',') {
			c->json++;
			lept_parse_whitespace(c);
		}
		else if (*c->json==']') {
			c->json++;
			v->type = LEPT_ARRAY;
			v->u.a.size = size;
			size *= sizeof(lept_value);  
			memcpy(v->u.a.e = (lept_value*)malloc(size), lept_context_pop(c, size), size);
			return LEPT_PARSE_OK;
		}
		else {
			ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	/* Pop and free values on the stack */
	for (i = 0; i < size; i++)
		lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
	return ret;
}

/* 解析对象 */
static int lept_parse_object(lept_context* c, lept_value* v) {
	size_t i, size;
	lept_member m;
	int ret;
	EXPECT(c, '{');
	lept_parse_whitespace(c);
	if (*c->json == '}') {
		c->json++;
		v->type = LEPT_OBJECT;
		v->u.o.m = 0;
		v->u.o.size = 0;
		return LEPT_PARSE_OK;
	}
	m.k = NULL;
	size = 0;
	for (;;){
		char* str;
		lept_init(&m.v);
		/* parse key to m.k, m.klen */
		if (*c->json != '"') {
			ret = LEPT_PARSE_MISS_KEY;
			break;
		}
		if ((ret=lept_parse_string_raw(c,&str,&m.klen))!=LEPT_PARSE_OK) {
			break;
		}
		memcpy(m.k = (char*)malloc(m.klen + 1), str, m.klen);
		m.k[m.klen] = '\0';
		/* parse ws colon ws */
		lept_parse_whitespace(c);
		if (*c->json != ':') {
			ret = LEPT_PARSE_MISS_COLON;
			break;
		}
		c->json++;
		lept_parse_whitespace(c);
		/* parse value */
		if ((ret = lept_parse_value(c, &m.v)) != LEPT_PARSE_OK) {
			break;
		}
		memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));
		size++;
		m.k = NULL;/* ownership is transferred to member on stack */
        /* parse ws [comma | right-curly-brace] ws */
		lept_parse_whitespace(c);
		if (*c->json == ',') {
			c->json++;
			lept_parse_whitespace(c);
		}
		else if (*c->json == '}') {
			size_t s = sizeof(lept_member) * size;
			c->json++;
			v->type = LEPT_OBJECT;
			v->u.o.size = size;
			memcpy(v->u.o.m = (lept_member*)malloc(s), lept_context_pop(c, s), s);
			return LEPT_PARSE_OK;
		}
		else {
			ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			break;
		}
	}
	/* Pop and free members on the stack */
	free(m.k);
	for (i = 0; i < size; i++) {
		lept_member* m = (lept_member*)lept_context_pop(c, sizeof(lept_member));
		free(m->k);
		lept_free(&m->v);
	}
	v->type = LEPT_NULL;
	return ret;
}

/* 解析 josn 值 */
static int lept_parse_value(lept_context* c, lept_value* v) {
	switch (*c->json) {
		case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
		case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
		case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
		default:  return lept_parse_number(c, v);
		case '"': return lept_parse_string(c, v);
		case '[': return lept_parse_array(c, v);
		case '{': return lept_parse_object(c, v);
		case '\0': return LEPT_PARSE_EXPECT_VALUE;   
	}
}

/* 解析 josn 字符串 */
int lept_parse(lept_value* v, const char* json) {
	lept_context c;
	int ret;
	assert(v != NULL);
	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;
	lept_init(v);
	lept_parse_whitespace(&c);
	ret = lept_parse_value(&c, v);
	if (ret == LEPT_PARSE_OK) {
		lept_parse_whitespace(&c);
		if (*c.json != '\0') {
			v->type = LEPT_NULL; 
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	assert(c.top==0);
	free(c.stack);
	return ret;
}

/* 生成字符串 */
static void lept_stringify_string(lept_context* c, const char* s, size_t len) {
	static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	size_t i, size;
	char* head, * p;
	assert(s != NULL);
	p = head = (char*)lept_context_push(c, size = len * 6 + 2); /* "\u00xx..." */
	*p++ = '"';
	for (i = 0; i < len; i++) {
		unsigned char ch = (unsigned char)s[i];
		switch (ch) {
		case '\"': *p++ = '\\'; *p++ = '\"'; break;
		case '\\': *p++ = '\\'; *p++ = '\\'; break;
		case '\b': *p++ = '\\'; *p++ = 'b';  break;
		case '\f': *p++ = '\\'; *p++ = 'f';  break;
		case '\n': *p++ = '\\'; *p++ = 'n';  break;
		case '\r': *p++ = '\\'; *p++ = 'r';  break;
		case '\t': *p++ = '\\'; *p++ = 't';  break;
		default:
			if (ch < 0x20) {
				*p++ = '\\'; *p++ = 'u'; *p++ = '0'; *p++ = '0';
				*p++ = hex_digits[ch >> 4];
				*p++ = hex_digits[ch & 15];
			}
			else
				*p++ = s[i];
		}
	}
	*p++ = '"';
	c->top -= size - (p - head);
}

/* 生成 json 值 */
static void lept_stringify_value(lept_context* c, const lept_value* v) {
	size_t i;
	switch (v->type) {
	case LEPT_NULL:   PUTS(c, "null", 4); break;
	case LEPT_FALSE:  PUTS(c, "false", 5); break;
	case LEPT_TRUE:   PUTS(c, "true", 4); break;
	case LEPT_NUMBER: c->top -= 32 - sprintf((char*)lept_context_push(c, 32),  "%.17g", v->u.n); break;   
	case LEPT_STRING: lept_stringify_string(c, v->u.s.s, v->u.s.len); break;
	case LEPT_ARRAY:
		PUTC(c, '[');
		for (i = 0; i < v->u.a.size; i++) {
			if (i > 0)
				PUTC(c, ',');
			lept_stringify_value(c, &v->u.a.e[i]);
		}
		PUTC(c, ']');
		break;
	case LEPT_OBJECT:
		PUTC(c, '{');
		for (i = 0; i < v->u.o.size; i++) {
			if (i > 0)
				PUTC(c, ',');
			lept_stringify_string(c, v->u.o.m[i].k, v->u.o.m[i].klen);
			PUTC(c, ':');
			lept_stringify_value(c, &v->u.o.m[i].v);
		}
		PUTC(c, '}');
		break;
	default: assert(0 && "invalid type");
	}
}

/* json 生成器：把树形数据结构转换为 json 文本，也成为字符串化。
length 参数是可选的，它会存储 json 的长度，传入 NULL 可忽略此参数。使用方需负责用 free() 释放内存。
为了简单起见，生成的 json 文本不做换行、缩进等美化处理，因此生成的 json 会是单行、无空白字符的最紧凑形式。 */
char* lept_stringify(const lept_value* v, size_t* length) {
	lept_context c;
	assert(v != NULL);
	c.stack = (char*)malloc(c.size = LEPT_PARSE_STRINGIFY_INIT_SIZE);
	c.top = 0;
	lept_stringify_value(&c, v);
	if (length) {
		*length = c.top;
	}
	PUTC(&c, '\0');
	return c.stack;
}

/* 深拷贝：把一个 json 复制一个版本出来修改，保持原来的不变 */
void lept_copy(lept_value* dst, const lept_value* src) {
	assert(src != NULL && dst != NULL && src != dst);
	size_t i;
	switch (src->type) {
		case LEPT_STRING:
			// 字符串类型直接进行拷贝
			lept_set_string(dst, src->u.s.s, src->u.s.len);
			break;
		case LEPT_ARRAY:// 数组
			// 先设置数组的容量
			lept_set_array(dst, src->u.a.size);
			// 逐个拷贝
			for (i = 0; i < src->u.a.size; i++) {
				lept_copy(&dst->u.a.e[i], &src->u.a.e[i]);
			}
			// 再设置大小
			dst->u.a.size = src->u.a.size;
			break;
		case LEPT_OBJECT:// 对象
			// 先设置容量
			lept_set_object(dst, src->u.o.size);
			// 逐个拷贝, 先 key 后 value
			for (i = 0; i < src->u.o.size; i++) {
				// key
				// 设置k字段为key的对象的值，如果在查找过程中找到了已经存在key，则返回；否则新申请一块空间并初始化，然后返回
				lept_value* val = lept_set_object_value(dst, src->u.o.m[i].k, src->u.o.m[i].klen);
				// value
				lept_copy(val, &src->u.o.m[i].v);
			}
			// 再设置大小
			dst->u.o.size = src->u.o.size;
			break;
		default:
			// 默认类型为 null、false、true，先释放 v 的内存，然后将 src 拷贝到 dst 中
			lept_free(dst);
			memcpy(dst, src, sizeof(lept_value));
			break;
	}
}

/* 模仿 c++11 的右值引用功能，也就是 复制+移动 操作 */
void lept_move(lept_value* dst, lept_value* src) {
	assert(dst != NULL && src != NULL && src != dst);
	// 释放 dst 的内存，然后把 src 的内容拷贝到 dst 中，最后将 scr 设置为 null
	lept_free(dst);
	memcpy(dst, src, sizeof(lept_value));
	lept_init(src);
}

/* 交换两个值 */
void lept_swap(lept_value* lhs, lept_value* rhs) {
	assert(lhs != NULL && rhs != NULL);
	if (lhs != rhs) {
		lept_value temp;
		memcpy(&temp, lhs, sizeof(lept_value));
		memcpy(lhs, rhs, sizeof(lept_value));
		memcpy(rhs, &temp, sizeof(lept_value));
	}
}

/* 释放内存 */
void lept_free(lept_value* v) {
	size_t i;
	assert(v != NULL);
	switch (v->type) {
	case LEPT_STRING:
		free(v->u.s.s);
		break;
	case LEPT_ARRAY:
		for (i = 0; i < v->u.a.size; i++)
			lept_free(&v->u.a.e[i]);
		free(v->u.a.e);
		break;
	case LEPT_OBJECT:
		for (i = 0; i < v->u.o.size; i++) {
			free(v->u.o.m[i].k);
			lept_free(&v->u.o.m[i].v);
		}
		free(v->u.o.m);
		break;
	default: break;
	}
	v->type = LEPT_NULL;
}

/* 获得 json 值的类型 */
lept_type lept_get_type(const lept_value* v) {
	assert(v != NULL);
	return v->type;
}

/* 比较两个 json 对象是否相等 */
int lept_is_equal(const lept_value* lhs, const lept_value* rhs) {
	size_t i;
	assert(lhs != NULL && rhs != NULL);
	if (lhs->type != rhs->type)// 类型不一样，直接返回 0
		return 0;
	// 对于 true、false、null 这三种类型，比较类型后便完成比较。而对于数组、对象、数字、字符串，需要进一步检查是否相等
	switch (lhs->type) {
		case LEPT_STRING:
			return lhs->u.s.len == rhs->u.s.len &&
				memcmp(lhs->u.s.s, rhs->u.s.s, lhs->u.s.len) == 0;
		case LEPT_NUMBER:
			return lhs->u.n == rhs->u.n;
		case LEPT_ARRAY:
			// 对于数组，首先比较元素的数目是否相等
			if (lhs->u.a.size != rhs->u.a.size)
				return 0;
			// 然后递归检查对应的元素是否相等
			for (i = 0; i < lhs->u.a.size; i++)
				if (!lept_is_equal(&lhs->u.a.e[i], &rhs->u.a.e[i]))
					return 0;
			return 1;
		case LEPT_OBJECT:
			// 对于对象，先比较键值对个数是否一样
			if (lhs->u.o.size != rhs->u.o.size)
				return 0;
			// 一样的话，对左边的键值对，依次在右边进行寻找
			/* key-value comp */
			for (i = 0; i < lhs->u.o.size; i++) {
				// 根据左边对象的键值对，在右边对象中寻找相对应的索引
				size_t index = lept_find_object_index(rhs, lhs->u.o.m[i].k, lhs->u.o.m[i].klen);
				if (index == LEPT_KEY_NOT_EXIST) { 
					return 0;// key 不存在直接返回 0
				}
				// 左右对象的 value 值不相等，直接返回 0
				if (!lept_is_equal(&lhs->u.o.m[i].v, &rhs->u.o.m[index].v))return 0;
			}
			return 1;
		default:
			return 1;
	}
}

/* 判断 v 的类型是否为 true */
int lept_get_boolean(const lept_value* v) {
	assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
	return v->type == LEPT_TRUE;
}

/* 根据 b 的值来设置 v 是 true 还是 false */
void lept_set_boolean(lept_value* v, int b) {
	lept_free(v);
	v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

/* 获得解析出来的数字 */
double lept_get_number(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}

/* 将浮点数 n 写入 v 中 */
void lept_set_number(lept_value* v, double n) {
	lept_free(v);
	v->u.n = n;
	v->type = LEPT_NUMBER;
}

/* 获得解析出来的字符串 */
const char* lept_get_string(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.s;
}

/* 获得解析出来的字符串长度 */
size_t lept_get_string_length(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.len;
}

/* 将字符串复制到 v 中 */
void lept_set_string(lept_value* v, const char* s, size_t len) {
	assert(v != NULL && (s != NULL || len == 0));
	lept_free(v);
	v->u.s.s = (char*)malloc(len + 1);
	memcpy(v->u.s.s, s, len);
	v->u.s.s[len] = '\0';
	v->u.s.len = len;
	v->type = LEPT_STRING;
}

/* 给数组分配内存 */
void lept_set_array(lept_value* v, size_t capacity) {
	assert(v != NULL);
	lept_free(v);// 释放 v 的内存
	// 设置 v 的类型为数组，大小设置为 0，容量设置为给定的容量，给数组的 lept_value 也分配对应大小的内存
	v->type = LEPT_ARRAY;
	v->u.a.size = 0;
	v->u.a.capacity = capacity;
	v->u.a.e = capacity > 0 ? (lept_value*)malloc(capacity * sizeof(lept_value)) : NULL;
}

/* 获得数组中的元素个数 */
size_t lept_get_array_size(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	return v->u.a.size;
}

/* 获得数组的容量 */
size_t lept_get_array_capacity(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	return v->u.a.capacity;
}

/* 将数组容量扩充到capacity，如果原来数组容量比capacity大，则无操作 */
void lept_reserve_array(lept_value* v, size_t capacity) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	// 当前容量不够时，需要扩大容量，使用标准库中的 realloc() 可以分配新的内存，并把旧的数据拷贝过去
	if (v->u.a.capacity < capacity) {
		v->u.a.capacity = capacity;
		v->u.a.e = (lept_value*)realloc(v->u.a.e, capacity * sizeof(lept_value));
	}
}

/* 当数组不需要修改时，可以把容量缩小至刚好能放置现有元素 */
void lept_shrink_array(lept_value* v) {   
	assert(v != NULL && v->type == LEPT_ARRAY);
	// 数组当前的容量大于数组当前的大小，则将数组容量缩小至数组的大小，然后使用 realloc() 重新分配内存，然后将旧的数据拷贝过去
	if (v->u.a.capacity > v->u.a.size) {
		v->u.a.capacity = v->u.a.size;
		v->u.a.e = (lept_value*)realloc(v->u.a.e, v->u.a.capacity * sizeof(lept_value));
	}
}

/* 清除数组中的所有元素，但是不改变容量 */
void lept_clear_array(lept_value* v) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	lept_erase_array_element(v, 0, v->u.a.size);
}

/* 根据索引访问数组中某个元素 */
lept_value* lept_get_array_element(const lept_value*v,size_t index) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	assert(index < v->u.a.size);
	return &v->u.a.e[index];
}

/* 在数据末端添加一个元素，然后返回新的元素指针 */
lept_value* lept_pushback_array_element(lept_value* v) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	// 若现有的容量不足，则使用 lept_reserve_array() 进行扩容，使用一个最简单的扩容公式：若容量为 0，则分配 1 个元素；其他情况就倍增容量。
	if (v->u.a.size == v->u.a.capacity)
		lept_reserve_array(v, v->u.a.capacity == 0 ? 1 : v->u.a.capacity * 2);
	lept_init(&v->u.a.e[v->u.a.size]);
	return &v->u.a.e[v->u.a.size++];
}

/* 删除最后一个元素，同时使用 lept_free() 去进行删除元素 */
void lept_popback_array_element(lept_value* v) {
	assert(v != NULL && v->type == LEPT_ARRAY && v->u.a.size > 0);
	lept_free(&v->u.a.e[--v->u.a.size]);
}

/* 在 index 位置插入一个元素 */
lept_value* lept_insert_array_element(lept_value* v, size_t index) {
    // index不可以超过size（因为是插入）（等于的话，相当于插在末尾）
	assert(v != NULL && v->type == LEPT_ARRAY && index <= v->u.a.size);
	// 若现有的容量不足，则使用 lept_reserve_array() 进行扩容，使用一个最简单的扩容公式：若容量为 0，则分配 1 个元素；其他情况就倍增容量。
	if (v->u.a.size == v->u.a.capacity){
		lept_reserve_array(v, v->u.a.capacity == 0 ? 1 : (v->u.a.size << 1)); //扩容为原来一倍
	}
	// 将 [index...size]之间的元素移动到 [index+1...size+1] 这个区间上，然后再 a[index] 上插入新元素的值
	memcpy(&v->u.a.e[index + 1], &v->u.a.e[index], (v->u.a.size - index) * sizeof(lept_value));
	// 初始化新插入元素，将大小+1，然后返回新插入元素
	lept_init(&v->u.a.e[index]);
	v->u.a.size++;
	return &v->u.a.e[index];
}

/* 删除在 index 位置开始共 count 个元素（不改容量） */
void lept_erase_array_element(lept_value* v, size_t index, size_t count) {
	assert(v != NULL && v->type == LEPT_ARRAY && index + count <= v->u.a.size);
	size_t i;
	// 回收完空间，然后将 index 后面 count 个元素移到 index 处，最后将空闲的 count 个元素重新初始化
	// 将[index,...,index+count-1]这些元素的空间进行释放
	for (i = index; i < index + count; i++) {
		lept_free(&v->u.a.e[i]);
	}
	// 将[index+count,...,size-1]之间的所有元素移动到 index 开始的位置处
	memcpy(v->u.a.e + index, v->u.a.e + index + count, (v->u.a.size - index - count) * sizeof(lept_value));
	// 将最后空间的 count 个元素进行重新初始化
	for (i = v->u.a.size - count; i < v->u.a.size; i++)
		lept_init(&v->u.a.e[i]);
	v->u.a.size -= count;
}

/* 根据给定大小初始化一个对象 */
void lept_set_object(lept_value* v, size_t capacity) {
	assert(v != NULL);
	lept_free(v);// 释放 v 的内存
	v->type = LEPT_OBJECT;
	v->u.o.size = 0;
	v->u.o.capacity = capacity;
	v->u.o.m = capacity > 0 ? (lept_member*)malloc(capacity * sizeof(lept_member)) : NULL;
}

/* 获得对象的元素个数 */
size_t lept_get_object_size(const lept_value*v) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	return v->u.o.size;
}

/* 返回对象容量大小 */
size_t lept_get_object_capacity(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	return v->u.o.capacity;
}

/* 将数组容量扩充到capacity，如果原来数组容量比capacity大，则无操作 */
void lept_reserve_object(lept_value* v, size_t capacity) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	if (v->u.o.capacity < capacity) {
		v->u.o.capacity = capacity;
		v->u.o.m = (lept_member*)realloc(v->u.o.m, capacity * sizeof(lept_member));
	}
}

/* 收缩容量到刚好符合大小 */
/* 当数组不需要修改时，可以把容量缩小至刚好能放置现有元素 */
void lept_shrink_object(lept_value* v) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	if (v->u.o.capacity > v->u.o.size) {
		v->u.o.capacity = v->u.o.size;
		v->u.o.m = (lept_member*)realloc(v->u.o.m, v->u.o.capacity * sizeof(lept_member));
	}
}

/* 清空对象 */
void lept_clear_object(lept_value* v) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	size_t i;
	for (i = 0; i < v->u.o.size; i++) {
		// 回收 key 和 value 空间
		free(v->u.o.m[i].k);
		v->u.o.m[i].k = NULL;
		v->u.o.m[i].klen = 0;
		lept_free(&v->u.o.m[i].v);
	}
	v->u.o.size = 0;
}

/* 获得对象的 key 值 */
const char* lept_get_object_key(const lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	assert(index < v->u.o.size);
	return v->u.o.m[index].k;
}

/* 获得对象 key 值的长度 */
size_t lept_get_object_key_length(const lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	assert(index < v->u.o.size);
	return v->u.o.m[index].klen;
}

/* 获得对象的 value 值 */
lept_value* lept_get_object_value(lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	assert(index < v->u.o.size);
	return &v->u.o.m[index].v;
}

/* 根据 key 值来获得它在对象中的索引值，时间复杂度 O(n) */
size_t lept_find_object_index(const lept_value* v, const char* key, size_t klen) {
	size_t i;
	assert(v != NULL && v->type == LEPT_OBJECT && key != NULL);
	for (i = 0; i < v->u.o.size; i++)
		if (v->u.o.m[i].klen == klen && memcmp(v->u.o.m[i].k, key, klen) == 0)
			return i;
	return LEPT_KEY_NOT_EXIST;
}

/* 根据 key 值获得其对应的 value 值 */
lept_value* lept_find_object_value(lept_value* v, const char* key, size_t klen) {
	size_t index = lept_find_object_index(v, key, klen);
	return index != LEPT_KEY_NOT_EXIST ? &v->u.o.m[index].v : NULL;
}

/* 设置 k 字段为 key 的对象的值，如果在查找过程中找到了已经存在key，则返回；否则新申请一块空间并初始化，然后返回。返回值为新增加键值对的值指针 */
lept_value* lept_set_object_value(lept_value* v, const char* key, size_t klen) {
	assert(v != NULL && v->type == LEPT_OBJECT && key != NULL);
	size_t i, index;
	index = lept_find_object_index(v, key, klen);
	if (index != LEPT_KEY_NOT_EXIST)
		return &v->u.o.m[index].v;
	// key not exist, then we make room and init
	if (v->u.o.size == v->u.o.capacity) {
		lept_reserve_object(v, v->u.o.capacity == 0 ? 1 : (v->u.o.capacity << 1));
	}
	i = v->u.o.size;
	// 申请 key 值的空间，并初始化 key，同时将最后一个字符设置为字符串结束的空字符
	v->u.o.m[i].k = (char*)malloc((klen + 1));
	memcpy(v->u.o.m[i].k, key, klen);
	v->u.o.m[i].k[klen] = '\0';
	v->u.o.m[i].klen = klen;
	// 初始化 v 值，然后将对象的大小+1，最后返回 v 值
	lept_init(&v->u.o.m[i].v);
	v->u.o.size++;
	return &v->u.o.m[i].v;
}

/* 根据索引删除一个对象 */
void lept_remove_object_value(lept_value* v, size_t index) {
	assert(v != NULL && v->type == LEPT_OBJECT && index < v->u.o.size);
	/* \todo */
	free(v->u.o.m[index].k);
	lept_free(&v->u.o.m[index].v);
	// think like a list
	memcpy(v->u.o.m + index, v->u.o.m + index + 1, (v->u.o.size - index - 1) * sizeof(lept_member));   // 这里原来有错误
	// 原来的size比如是10，最多其实只能访问下标为9
	// 删除一个元素，再进行挪移，原来为9的地方要清空
	// 现在先将size--，则size就是9
	v->u.o.m[--v->u.o.size].k = NULL;  
	v->u.o.m[v->u.o.size].klen = 0;
	lept_init(&v->u.o.m[v->u.o.size].v);
	// 
	// 错误，因为无法访问原来的size下标
	//v->u.o.m[v->u.o.size].k = NULL;
	//printf("v->u.o.size：  %d\n", v->u.o.size);
	//printf("v->u.o.m[v->u.o.size]：  %s\n", v->u.o.m[v->u.o.size]);
	//printf("v->u.o.m[v->u.o.size].klen： %d\n", v->u.o.m[v->u.o.size].klen);
	//v->u.o.m[v->u.o.size].klen = 0;
	//lept_init(&v->u.o.m[--v->u.o.size].v);
}