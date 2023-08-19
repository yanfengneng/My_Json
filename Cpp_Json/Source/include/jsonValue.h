#ifndef VALUE_H__
#define VALUE_H__

#include <string>
#include <vector>
#include <utility>
#include "json.h"

namespace miniJson
{
    class JsonValue final
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
        const JsonValue& get_array_element(size_t index) const noexcept;
        void set_array(const std::vector<JsonValue> &arr) noexcept;
        void pushback_array_element(const JsonValue& val) noexcept;
        void popback_array_element() noexcept;
        void insert_array_element(const JsonValue &val, size_t index) noexcept;
        void erase_array_element(size_t index, size_t count) noexcept;
        void clear_array() noexcept;

        /* 对对象操作 */
        size_t get_object_size() const noexcept;
        const std::string& get_object_key(size_t index) const noexcept;
        size_t get_object_key_length(size_t index) const noexcept;
        const JsonValue& get_object_value(size_t index) const noexcept;
        void set_object_value(const std::string &key, const JsonValue &val) noexcept;
        void set_object(const std::vector<std::pair<std::string, JsonValue>> &obj) noexcept;
        long long find_object_index(const std::string &key) const noexcept;
        void remove_object_value(size_t index) noexcept;
        void clear_object() noexcept;

        JsonValue() noexcept { num_ = 0; }
        JsonValue(const JsonValue &rhs) noexcept { init(rhs); }
        JsonValue& operator=(const JsonValue &rhs) noexcept;
        ~JsonValue() noexcept;
    private:
        void init(const Value &rhs) noexcept;
        void free() noexcept;

        miniJson::JsonType type_ = miniJson::Null;
        union {
            double num_;
            std::string str_;
            std::vector<JsonValue> arr_;
            std::vector<std::pair<std::string, JsonValue>> obj_;
        };

        friend bool operator==(const JsonValue &lhs, const JsonValue &rhs) noexcept;
    
    };

    bool operator==(const JsonValue &lhs, const JsonValue &rhs) noexcept;
    bool operator!=(const JsonValue &lhs, const JsonValue &rhs) noexcept;
} // namespace miniJson

#endif