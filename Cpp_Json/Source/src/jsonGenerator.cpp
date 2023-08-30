#include "jsonGenerator.h"
#include <cassert>
namespace yfn
{
    namespace json
    {
        /* 生成器的构造函数 */
        Generator::Generator(const Value& val, std::string& result) : res_(result){
            res_.clear();
            stringify_value(val);
        }

        /* 生成 json 值 */
        void Generator::stringify_value(const Value& v){
            switch(v.get_type()) {
                case json::Null: res_ += "null"; break;
                case json::True: res_ += "true"; break;
                case json::False: res_ += "false"; break;
                case json::Number:{
                        char buffer[32] = {0};
                        sprintf(buffer, "%.17g", v.get_number());
                        res_ += buffer;
                    }
                    break;
                case json::String: stringify_string(v.get_string());// 生成字符串
                    break;
                // 生成数组：只要输出"[]"，中间对逐个子值递归调用 stringify_value()
                case json::Array:
                    res_ += '[';
                    for(size_t i = 0; i < v.get_array_size(); i++){
                        if (i > 0) res_ += ',';
                        stringify_value(v.get_array_element(i));
                    }
                    res_ += ']';
                    break;
                // 生成对象
                case json::Object:
                    res_ += '{';
                    for (int i = 0; i < v.get_object_size(); ++i) {
                        if (i > 0) res_ += ',';
                        // 对象需要多处理一个 key 和冒号
                        stringify_string(v.get_object_key(i));
                        res_ += ':';
                        // 递归调用生成 json 值
                        stringify_value(v.get_object_value(i));
                    }
                    res_ += '}';
                    break;
                default: assert(0 && "invalid type");
            }
        }

        /* 生成字符串 */
        void Generator::stringify_string(const std::string &str){
            res_ += '\"';
            for(auto it = str.begin(); it != str.end(); it++){
                unsigned char ch = *it;
                switch (ch)
                {
                    /* 添加这些转义字符 */
                    case '\"': res_ += "\\\""; break;
                    case '\\': res_ += "\\\\"; break;
                    case '\b': res_ += "\\b";  break;
                    case '\f': res_ += "\\f";  break;
                    case '\n': res_ += "\\n";  break;
                    case '\r': res_ += "\\r";  break;
                    case '\t': res_ += "\\t";  break;
                    default:
                        // 低于 0x20 的字符需要转义为 \u00xx 的形式
                        if (ch < 0x20) {
                            char buffer[7] = {0};
                            sprintf(buffer, "\\u%04X", ch);
                            res_ += buffer;
                        }
                        else
                            res_ += *it;
                }
            }
            res_ += '\"';// 添加最后一个双引号
        }
    } // namespace json
    
} // namespace yfn
