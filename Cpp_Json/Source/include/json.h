#ifndef JSON_H__
#define JSON_H__

#include <memory>
#include <string>

namespace yfn
{
    namespace json
    {
        /* 枚举数据类型 */
        enum type : int{Null, True, False, Number, String, Array, Object};

        /* 前向声明 */
        class Value;
    } // namespace json
    
    /* Json 类负责提供接口，Value 类负责实现接口，Json 类通过调用一个 std::unique_ptr 的智能指针实现对 Value 的访问 */
    class Json final
    {
        /* 解析 json 字符串 */
        void parse(const std::string &content, std::string &status) noexcept;
        void parse(const std::string &content);

        /* 生成 json 字符串 */
        void stringify(std::string &content) const noexcept;

        /* json 类的构造函数 */
        Json() noexcept;
        ~Json() noexcept;
        Json(const Json &rhs) noexcept;             // 深拷贝构造函数
        Json& operator=(const Json &rhs) noexcept;  // 赋值拷贝构造函数
        Json(Json &&rhs) noexcept;                  // 移动拷贝构造函数
        Json& operator=(Json &&rhs) noexcept;       // 赋值移动拷贝构造函数
        void swap(Json &rhs) noexcept;              // 交换

        /* 对 null、true、false 的操作 */
        int get_type() const noexcept;
        void set_null() noexcept;
        void set_boolean(bool b) noexcept;
        Json& operator=(bool b) noexcept { set_boolean(b); return *this; }

        /* 对数字的操作*/
        double get_number() const noexcept;
        void set_number(double d) noexcept;
        Json& operator=(double d) noexcept { set_number(d); return *this; }

        /* 对字符串的操作 */
        const std::string get_string() const noexcept;
        void set_string(const std::string& str) noexcept;
        Json& operator=(const std::string& str) noexcept { set_string(str); return *this; }

        /* 对数组的操作 */
        void set_array() noexcept;
        size_t get_array_size() const noexcept;
        Json get_array_element(size_t index) const noexcept;
        void pushback_array_element(const Json& val) noexcept;
        void popback_array_element() noexcept;
        void insert_array_element(const Json &val, size_t index) noexcept;
        void erase_array_element(size_t index, size_t count) noexcept;
        void clear_array() noexcept;

        /* 对对象进行操作 */
        void set_object() noexcept;
        size_t get_object_size() const noexcept;
        const std::string& get_object_key(size_t index) const noexcept;
        size_t get_object_key_length(size_t index) const noexcept;
        Json get_object_value(size_t index) const noexcept;
        void set_object_value(const std::string &key, const Json &val) noexcept;
        long long find_object_index(const std::string &key) const noexcept;
        void remove_object_value(size_t index) noexcept;
        void clear_object() noexcept;
    private:
        /* Json 类只提供接口，Value 负责实现该接口 */
        std::unique_ptr<json::Value> v;

        /* 友元函数 */
        friend bool operator==(const Json &lhs, const Json &rhs) noexcept;
        friend bool operator!=(const Json &lhs, const Json &rhs) noexcept;
    };

    bool operator==(const Json &lhs, const Json &rhs) noexcept;
    bool operator!=(const Json &lhs, const Json &rhs) noexcept;
    void swap(Json &lhs, Json &rhs) noexcept;
}
#endif