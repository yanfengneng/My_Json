#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include<stddef.h> // size_t

typedef enum {
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT
} lept_type;

#define LEPT_KEY_NOT_EXIST ((size_t)-1)

typedef struct lept_value lept_value;
typedef struct lept_member lept_member;

struct lept_value {
	union {
		struct { lept_member* m; size_t size, capacity; }o;  // 增加了容量
		struct { lept_value* e; size_t size, capacity; } a; 
		struct { char* s; size_t len; } s;       
		double n;
	}u;
	lept_type type;
};

struct lept_member {
	char*k; size_t klen;
	lept_value v;
};

enum {
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG,
	LEPT_PARSE_MISS_QUOTATION_MARK,
	LEPT_PARSE_INVALID_STRING_ESCAPE,
	LEPT_PARSE_INVALID_STRING_CHAR,
	LEPT_PARSE_INVALID_UNICODE_HEX,
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
	LEPT_PARSE_MISS_KEY,
	LEPT_PARSE_MISS_COLON,
	LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

#define lept_init(v) do{(v)->type=LEPT_NULL;}while(0)

int lept_parse(lept_value* v, const char* json); 
char* lept_stringify(const lept_value* v, size_t* length);

// 复制、移动、交换
void lept_copy(lept_value* dst, const lept_value* src);
void lept_move(lept_value* dst, lept_value* src);
void lept_swap(lept_value* lhs, lept_value* rhs);

void lept_free(lept_value* v);

lept_type lept_get_type(const lept_value* v);

// 比较两个json值是否一样
int lept_is_equal(const lept_value* lhs, const lept_value* rhs);

#define lept_set_null(v) lept_free(v)

int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);


size_t lept_get_array_size(const lept_value* v);
lept_value* lept_get_array_element(const lept_value* v, size_t index);

void lept_set_array(lept_value* v, size_t capacity);  // 给数组分配内存
size_t lept_get_array_capacity(const lept_value* v);  // 获得数组内存
void lept_reserve_array(lept_value* v, size_t capacity);  // 将数组容量扩充到capacity，如果原来数组容量比 capacity 大，则无操作
void lept_shrink_array(lept_value* v);   // 收缩数组容量
void lept_clear_array(lept_value* v);    // 清空数组
lept_value* lept_pushback_array_element(lept_value* v); // 插入元素
void lept_popback_array_element(lept_value* v);  
lept_value* lept_insert_array_element(lept_value* v, size_t index);
void lept_erase_array_element(lept_value* v, size_t index, size_t count); // 删除元素


size_t lept_get_object_size(const lept_value* v);
const char* lept_get_object_key(const lept_value* v, size_t index);
size_t lept_get_object_key_length(const lept_value* v, size_t index);
lept_value* lept_get_object_value(lept_value* v, size_t index);

void lept_set_object(lept_value* v, size_t capacity);
size_t lept_get_object_capacity(const lept_value* v);
void lept_reserve_object(lept_value* v, size_t capacity);
void lept_shrink_object(lept_value* v);
void lept_clear_object(lept_value* v);
// 查询一个键值是否存在
size_t lept_find_object_index(const lept_value* v, const char* key, size_t klen);
lept_value* lept_find_object_value(lept_value* v, const char* key, size_t klen);
lept_value* lept_set_object_value(lept_value* v, const char* key, size_t klen);
void lept_remove_object_value(lept_value* v, size_t index);

#endif // !LEPTJSON_H__