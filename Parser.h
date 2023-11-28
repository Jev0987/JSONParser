#ifndef MYUTIL_PARSER_H
#define MYUTIL_PARSER_H

#include <string>
#include <string_view>
#include <sstream>
#include "JObject.h"
namespace json
{
#define FUNC_TO_NAME _to_json
#define FUNC_FROM_NAME _from_json

#define START_TO_JSON void FUNC_TO_NAME(json::JObject & obj) const{
#define to(key) obj[key]
    // push 一个自定义类型的成员
#define to_struct(key, struct_member) json::JObject tmp((json::dict_t()));struct_member.FUNC_TO_NAME(tmp);obj[key]=tmp
#define END_TO_JSON }

#define START_FROM_JSON void FUNC_FROM_NAME(json::JObject& obj){
#define from(key, type) obj[key].Value<type>()
#define from_struct(key, struct_member) struct_member.FUNC_FROM_NAME(obj[key])
#define END_FROM_JSON }

    using std::string;
    using std::string_view;
    using std::stringstream;

    class JObject;

    class Parser
    {
    public:
        Parser() = default;

        static JObject FromString(string_view content);  // JSON字符串转化为Jobject对象

        template <typename T>
        static string ToJSON(T const &src)
        {
            // 如果是基本类型
            if constexpr(IS_TYPE(T, int_t))
            {
                JObject obj(src);
                return obj.to_string();
            }else if constexpr(IS_TYPE(T, bool_t))
            {
                JObject obj(src);
                return obj.to_string();
            }else if constexpr(IS_TYPE(T, str_t))
            {
                JObject obj(src);
                return obj.to_string();
            }else if constexpr(IS_TYPE(T, double_t))
            {
                JObject obj(src);
                return obj.to_string();
            }
            json::JObject obj((json::dict_t()));
            src.FUNC_TO_NAME(obj);
            return obj.to_string();
        }

        template <typename T>
        static T FromJSON(string_view src)
        {
            JObject obj = FromString(src);
            // 如果是基本类型
            if constexpr(is_basic_type<T>())
            {
                // template明示编译器这是一个模版函数，
                // 表示obj对象上调用的Value函数是以T类型作为模版参数
                return obj.template Value<T>();
            }

            // 调用T类型对应的 xx 函数
            if(obj.Type() != T_DICT) throw std::logic_error("not dict type from json");
            T ret;
            ret.FUNC_FROM_NAME(obj);
            return ret;
        }

        void init(string_view src);
        void trim_right();  // 去除多余空格
        void skip_comment();  // 跳过注释
        bool is_esc_consume(size_t pos);  
        char get_next_token();  // 跳过空格和跳过注释

        JObject parse();
        JObject parse_null();
        JObject parse_number();
        bool parse_bool();
        string parse_string();
        JObject parse_list();
        JObject parse_dict();

    private:
        string m_str;
        size_t m_idx{};
    };
}
#endif