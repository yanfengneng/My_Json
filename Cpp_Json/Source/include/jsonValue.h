#ifndef JOSN_VALUE_H__
#define JOSN_VALUE_H__

#include <string>
#include <vector>
#include <utility>
#include "json.h"

namespace yfn
{
    namespace json
    {
        /* 实现对 json 值进行操作 */
        class Value final
        {
        public:
            /* 解析 json 字符串、序列化 json 字符串 */
            void parse(const std::string &content);
            void stringify(std::string &content) const noexcept;

            /* 对 null、false、true 操作 */
            int get_type() const noexcept;
            void set_type(type t) noexcept;

            /* 对数字操作 */
            double get_number() const noexcept;
            void set_number(double d) noexcept;

            /* 对字符串操作 */
            const std::string& get_string() const noexcept;
            void set_string(const std::string &str) noexcept;

            /* 对数组操作 */
            size_t get_array_size() const noexcept;
            const Value& get_array_element(size_t index) const noexcept;
            void set_array(const std::vector<Value> &arr) noexcept;
            void pushback_array_element(const Value& val) noexcept;
            void popback_array_element() noexcept;
            void insert_array_element(const Value &val, size_t index) noexcept;
            void erase_array_element(size_t index, size_t count) noexcept;
            void clear_array() noexcept;

            /* 对对象操作 */
            size_t get_object_size() const noexcept;
            const std::string& get_object_key(size_t index) const noexcept;
            size_t get_object_key_length(size_t index) const noexcept;
            const Value& get_object_value(size_t index) const noexcept;
            void set_object_value(const std::string &key, const Value &val) noexcept;
            void set_object(const std::vector<std::pair<std::string, Value>> &obj) noexcept;
            long long find_object_index(const std::string &key) const noexcept;
            void remove_object_value(size_t index) noexcept;
            void clear_object() noexcept;

            /* 构造函数与析构函数 */
            Value() noexcept { num_ = 0; }
            Value(const Value &rhs) noexcept { init(rhs); }
            Value& operator=(const Value &rhs) noexcept;
            ~Value() noexcept;
        private:
            /* 初始化 Value 与释放 Value 的内存 */
            void init(const Value &rhs) noexcept;
            void free() noexcept;

            json::type type_ = json::Null;

            /*
                由于 JSON 是一个树形结构，因此最终我们需要实现一个树的数据结构。
                由于一个 json 值不可能同时为数字和字符串，因此可以使用 union 来节约内存。
            */
            union 
            {
                double num_;
                std::string str_;
                std::vector<Value> arr_;
                std::vector<std::pair<std::string, Value>> obj_;
            };
            
        };

        /* 比较两个 json 值 */
        bool operator==(const Value &lhs, const Value &rhs) noexcept;
        bool operator!=(const Value &lhs, const Value &rhs) noexcept;
    }
} // namespace yfn

#endif