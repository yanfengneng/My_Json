#include "json.h"
#include "jsonValue.h"
#include "jsonException.h"

namespace yfn
{
    void Json::parse(const std::string& content, std::string& status)noexcept
    {
        try{
            parse(content);
            status = "parse ok!";
        }catch (const json::Exception& msg){
            status = msg.what();
        }catch(...){

        }
    }

    void Json::parse(const std::string &content){
        v-> parse(content);
    }

    /* 生成 json 字符串 */
    void Json::stringify(std::string &content) const noexcept{
        v-> stringify(content);
    }

    /* json 类的构造函数 */
    Json::Json() noexcept : v(new json::Value) { }
    Json::~Json() noexcept { }
    Json::Json(const Json &rhs) noexcept{// 深拷贝构造函数
        v.reset(new json::Value(*(rhs.v)));
    }             
    Json& Json::operator=(const Json &rhs) noexcept{// 赋值拷贝构造函数
        v.reset(new json::Value(*(rhs.v)));
        return *this;
    }  
    Json::Json(Json &&rhs) noexcept{// 移动拷贝构造函数
        v.reset(rhs.v.release());
    }                  
    Json& Json::operator=(Json &&rhs) noexcept{// 赋值移动拷贝构造函数
        v.reset(rhs.v.release());
        return *this;
    }      
    void Json::swap(Json &rhs) noexcept{// 交换
        using std::swap;
        swap(v,rhs.v);
    }             

    void swap(Json &lhs, Json &rhs) noexcept{
        lhs.swap(rhs);
    }

    /* 对 null、true、false 的操作 */
    int Json::get_type() const noexcept{
        if(v == nullptr)return json::Null;
        return v-> get_type();
    }

    void Json::set_null() noexcept{
        v-> set_type(json::Null);
    }

    void Json::set_boolean(bool b) noexcept{
        if(b) v-> set_type(json::True);
        else v-> set_type(json::False);
    }
    
    /* 对数字的操作*/
    double Json::get_number() const noexcept{
        return v-> get_number();
    }
    void Json::set_number(double d) noexcept{
        v-> set_number(d);
    }

    /* 对字符串的操作 */
    const std::string Json::get_string() const noexcept{
        return v-> get_string();
    }
    void Json::set_string(const std::string& str) noexcept{
        v-> set_string(str);
    }
    

    /* 对数组的操作 */
    void Json::set_array() noexcept{
        v-> set_array(std::vector<json::Value {}>);
    }

    size_t Json::get_array_size() const noexcept{
        return v-> get_array_size();
    }

    Json Json::get_array_element(size_t index) const noexcept{
        Json ret;
		ret.v.reset(new json::Value(v-> get_array_element(index)));
		return ret;
    }

    void Json::pushback_array_element(const Json& val) noexcept{
        v-> pushback_array_element(*val.v);
    }

    void Json::popback_array_element() noexcept{
        v-> popback_array_element();
    }

    void Json::insert_array_element(const Json &val, size_t index) noexcept{
        v-> insert_array_element(*val.v, index);
    }

    void Json::erase_array_element(size_t index, size_t count) noexcept{
        v-> erase_array_element(index, count);
    }

    void Json::clear_array() noexcept{
        v-> clear_array();
    }

    /* 对对象进行操作 */
    void Json::set_object() noexcept{
        v-> set_object(std::vector<std::pair<std::string, json::Value>> {});
    }
    size_t Json::get_object_size() const noexcept{
        return v-> get_object_size();
    }
    const std::string& Json::get_object_key(size_t index) const noexcept{
        return v-> get_object_key(index);
    }
    size_t Json::get_object_key_length(size_t index) const noexcept{
        return v-> get_object_key_length(index);
    }
    Json Json::get_object_value(size_t index) const noexcept{
        Json ret;
		ret.v.reset(new json::Value(v-> get_object_value(index)));
		return ret;
    }
    void Json::set_object_value(const std::string &key, const Json &val) noexcept{
        v-> set_object_value(key, *val.v);
    }
    long long Json::find_object_index(const std::string &key) const noexcept{
        return v-> find_object_index(key);
    }
    void Json::remove_object_value(size_t index) noexcept{
        v-> remove_object_value(index);
    }
    void Json::clear_object() noexcept{
        v-> clear_object();
    }

    bool operator==(const Json &lhs, const Json &rhs) noexcept
	{
		return *lhs.v == *rhs.v;
    }

   	bool operator!=(const Json &lhs, const Json &rhs) noexcept
	{
		return *lhs.v != *rhs.v;
	}
    /* 两个友元函数的实现 */
} // namespace yfn
