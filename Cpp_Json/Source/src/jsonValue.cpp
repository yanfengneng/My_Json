#include <assert.h>
#include <string>
#include "jsonValue.h"
#include "jsonParser.h"
#include "jsonGenerator.h"

namespace yfn
{
    namespace json
    {
        /* 构造函数 */
        Value& Value::operator=(const Value &rhs) noexcept;
        {
            free();
            init(rhs);
        }

        /* 析构函数 */
        Value::~Value() noexcept
        {
            free();
        }

        /* 初始化 Value 的内存 */
        void Value::init(const Value &rhs) noexcept
        {
            type_ = rhs.type_;
            num_ = 0;
            switch (type_)
            {
            case json::Number: num_ = rhs.num_;
                break;
            case json::String: new(&str_) std::string(rhs.str_);
                break;
            case json::Array: new(&arr_) std::vector<Value>(rhs.arr_);
                break;
            case json::Object: new(&obj_) std::vector<std::pair<std::string, Value>> (rhs.obj_);
                break;
            }
        }

        /* 释放 Value 的内存 */
        void Value::free() noexcept
        {
            using std::string;
            switch (type_)
            {
            case json::String: str_.~string(); // 显示调用相应的析构函数
                break;
            case json::Array: arr_.~vector<Value>();
                break;
            case json::Object: obj_.~vector<std::pair<std::string, Value>>();
                break;
            }
        }

        /* 解析 json 字符串 */
        void Value::parse(const std::string &content){
            Parse(*this, content);
        }

        /* 序列化 json 字符串 */
        void Value::stringify(std::string &content) const noexcept{
            Generator(*this, content);
        }

        /* 对 null、false、true 操作 */
        /* 获得 Value 的类型 */
        int Value::get_type() const noexcept{
            return type_;
        }

        /* 设置 Value 的类型 */
        void Value::set_type(type t) noexcept{
            // 先释放内存，然后再重置类型
            free();
            type_ = t;
        }

        /* 对数字操作 */
        /* 获得 Value 中解析出来的数字 */
        double Value::get_number() const noexcept{
            assert(type_ == json::Number);
            return num_;
        }
        /* 设置 Value 中的数字 */
        void Value::set_number(double d) noexcept{
            free();
            type_ = json::Number;
            num_ = d;
        }

        /* 对字符串操作 */
        /* 获得 Value 中解析出来的字符串 */
        const std::string& Value::get_string() const noexcept{
            assert(type_ == type::String);
            return str_;
        }
        /* 设置 Value 中的字符串 */
        void Value::set_string(const std::string &str) noexcept{
            if(type_ == json::String)
                str_ = str;
            else{
                // 释放内存，然后
                free();
                type_ = json::String;
                new(&str_) std::string(str);
            }
        }

        /* 对数组操作 */
        /* 获得数组的大小 */
        size_t Value::get_array_size() const noexcept{
            assert(type_ == json::Array);
            return arr_.size();
        }

        /* 根据索引获得数组中的元素 */
        const Value& Value::get_array_element(size_t index) const noexcept{
            assert(type_ == json::Array);
            return arr_[index];
        }

        /* 重置数组 */
        void Value::set_array(const std::vector<Value> &arr) noexcept{
            if(type_ == json::Array)
                arr_ = arr;
            else {
                free();
                type_ = json::Array;
                new(&arr_) std::vector<Value>(arr);
            }
        }

        /* 在数组末尾添加元素 */
        void Value::pushback_array_element(const Value& val) noexcept{
            assert(type_ == json::Array);
            arr_.push_back(val);
        }
        
        /* 删除数组的最后一个元素 */
        void Value::popback_array_element() noexcept{
            assert(type_ == json::Array);
            arr_.pop_back();
        }

        /* 根据索引在数组中的某个位置插入元素 */
        void Value::insert_array_element(const Value &val, size_t index) noexcept{
            assert(type_ == json::Array);
            arr_.insert(arr_.begin()+index, val);
        }

        /* 根据索引删除数组中的某段区间内的元素 */
        void Value::erase_array_element(size_t index, size_t count) noexcept{
            assert(type_ == json::Array);
            arr_.erase(arr_.begin()+index, arr_.begin()+index+count);
        }

        /* 清空数组 */
        void Value::clear_array() noexcept{
            assert(type_ == json::Array);
            arr_.clear();
        }

        /* 对对象操作 */
        /* 获得对象的大小 */
        size_t Value::get_object_size() const noexcept{
            assert(type_ == json::Object);
            return obj_.size();
        }

        /* 根据索引获得对象的 key 值 */
        const std::string& Value::get_object_key(size_t index) const noexcept{
            assert(type_ == json::Object);
            return obj_[index].first;
        }

        /* 根据索引获得该 key 值的长度 */
        size_t Value::get_object_key_length(size_t index) const noexcept{
            assert(type_ == json::Object);
            return obj_[index].first.size();
        }

        /* 根据索引获得对象的 value 值 */
        const Value& Value::get_object_value(size_t index) const noexcept{
            assert(type_ == json::Object);
            return obj_[index].second;
        }

        /* 根据 key 值设置该对象的 value 值 */
        void Value::set_object_value(const std::string &key, const Value &val) noexcept{
            assert(type_ == json::Object);
            // 若 key 值存在，则替换 key 值对应的 value；否则就添加新的一个键值对
            auto index = find_object_index(key);
            if(index >= 0)obj_[index].second = val;
            else obj_.push_back(make_pair(key, val));
        }

        /* 重置对象 */
        void Value::set_object(const std::vector<std::pair<std::string, Value>> &obj) noexcept{
            if(type_ == json::Object)
                obj_ = obj;
            else {
                free();
                type_ = json::Object;
                new(&obj_) std::vector<std::pair<std::string, Value>>(obj);
            }
        }

        /* 根据 key 值寻找该对象在数组中的索引号 */
        long long Value::find_object_index(const std::string &key) const noexcept{
            assert(type_ == json::Object);
            for(size_t i = 0, n = obj_.size(); i < n; ++i){
                if(obj_[i].first == key)
                    return i;
            }
            return -1;
        }

        /* 根据索引删除某个对象 */
        void Value::remove_object_value(size_t index) noexcept{
            assert(type_ == json::Object);
            obj_.erase(obj_.begin()+index, obj_.begin()+index+1);
        }

        /* 清空对象 */
        void Value::clear_object() noexcept{
            assert(type_ == json::Object);
            obj_.clear();
        }

        /* 比较两个 json 值 */
        bool operator==(const Value &lhs, const Value &rhs) noexcept{
            if(lhs.type_ != rhs.type_)
                return false;
	        // 对于 true、false、null 这三种类型，比较类型后便完成比较。而对于数组、对象、数字、字符串，需要进一步检查是否相等
            switch (lhs.type_)
            {
            case json::Number: return lhs.num_ == rhs.num_;
            case json::String: return lhs.str_ == rhs.str_;
            case json::Array: return lhs.arr_ == rhs.arr_;
            case json::Object:
                // 对于对象，先比较键值对的个数是否相等
                if(lhs.get_object_size() != rhs.get_object_size())
                    return false;
                // 相等的话，对左边的键值对，依次在右边进行寻找
                for(size_t i = 0, n = lhs.get_object_size(); i < n; i++){
                    // 根据左边对象的键值对，在右边对象中寻找相对应的索引
                    auto index = rhs.find_object_index(lhs.get_object_key(i));
                    // key 不存在或者左右对象的 value 值不相等，直接返回 0
                    if(index < 0 || lhs.get_object_value(i) != rhs.get_object_value(index))
                        return false;
                }
                return true;
            default:
                return true;
            }
        }

        bool operator!=(const Value &lhs, const Value &rhs) noexcept{
            return !(lhs == rhs);
        }
    }
} // namespace yfn
