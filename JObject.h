#ifndef MYUTIL_JOBJECT_H
#define MYUTIL_JOBJECT_H

#include <iostream>
// #include <utility>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <map>
#include <variant>
#include <sstream>

namespace json
{   
    // c++17中的变体类型，variant<X,Y,Z>是可存放X，Y，Z三种类型数据的变体类型
    // 和C的union相同的是：他也是union类型
    // 和C的union不同的是：他是可以辨识类型安全的union
    using std::variant;  
    using std::map;
    using std::string;
    using std::string_view;
    using std::stringstream;
    using std::vector;
    using std::get_if;  // 判断变体类型下是否有某数据类型，返回类型值的地址

    enum TYPE
    {
        T_NULL,
        T_BOOL,
        T_INT,
        T_DOUBLE,
        T_STR,
        T_LIST,
        T_DICT
    };

    class JObject;

    using null_t = string;
    using int_t = int32_t;
    using bool_t = bool;
    using double_t = double;
    using str_t = string;
    using list_t = vector<JObject>;
    using dict_t = map<string, JObject>;
    
    // c++11中 std::is_same可以判断输入的类型是否是指定的模版类型
    // std::is_same<T, int>::value
#define IS_TYPE(typea, typeb) std::is_same<typea, typeb>::value

    // 判断是否为基本类型
    template<typename T>
    constexpr bool is_basic_type()
    {
        if constexpr(IS_TYPE(T, str_t) ||
                     IS_TYPE(T, bool_t) ||
                     IS_TYPE(T, double_t) ||
                     IS_TYPE(T, int_t))
            return true;

        return false;
    }

    class JObject
    {
    public:
        using value_t = variant<bool_t, int_t, double_t, str_t, list_t, dict_t>;

        JObject()// 默认为null类型
        {
            m_type = T_NULL;
            m_value = "null";
        }
        JObject(int_t value)
        {
            Int(value);
        }

        JObject(bool_t value)
        {
            Bool(value);
        }

        JObject(double_t value)
        {
            Double(value);
        }

        JObject(str_t const &value)
        {
            Str(value);
        }

        JObject(list_t value)
        {
            List(std::move(value));
        }

        JObject(dict_t value)
        {
            Dict(std::move(value));
        }


        void Null()
        {
            m_type = T_NULL;
            m_value = "null";
        }

        void Int(int_t value)
        {
            m_type = T_INT;
            m_value = value;
        }

        void Bool(bool_t value)
        {
            m_type = T_BOOL;
            m_value = value;
        }

        void Double(double_t value)
        {
            m_type = T_DOUBLE;
            m_value = value;
        }

        void Str(str_t value)
        {
            m_type = T_STR;
            m_value = value;
        }

        void List(list_t value)
        {
            m_type = T_LIST;
            m_value = std::move(value);  // 将左值转换为右值直接赋给m_value
        }

        void Dict(dict_t value)
        {
            m_type = T_DICT;
            m_value = std::move(value);
        }
#define THROW_GET_ERROR(erron) throw std::logic_error("type error in get"#erron" value!")

    template<typename V>
    V &Value()
    {
        // 添加安全检查
        if constexpr(IS_TYPE(V, str_t))
        {
            if(m_type != T_STR)
                THROW_GET_ERROR(string);
        }else if constexpr(IS_TYPE(V, bool_t))
        {
            if(m_type != T_BOOL)
                THROW_GET_ERROR(BOOL);
        }else if constexpr(IS_TYPE(V, double_t))
        {
            if(m_type != T_DOUBLE)
                THROW_GET_ERROR(DOUBLE);
        }else if constexpr(IS_TYPE(V, list_t))
        {
            if(m_type != T_LIST)
                THROW_GET_ERROR(LIST);
        }else if constexpr(IS_TYPE(V, dict_t))
        {
            if(m_type != T_DICT)
                THROW_GET_ERROR(DICT);
        }

        void *v = value();  // 指向对象值的指针，用void* 的原因是因为该函数返回的是variant类型的值
        if(v == nullptr)
            throw std::logic_error("unknown type in JObject::Value()");
        return *((V *) v);  // void*类型转换为V*类型指针，然后通过*获取该指针指向的值
    }

    TYPE Type()
    {
        return m_type;
    }

    string to_string();

    void push_back(JObject item)
    {
        if(m_type == T_LIST)
        {
            auto &list = Value<list_t>();
            list.push_back(std::move(item));
            return;
        }
        throw std::logic_error("not a list type! JObject::push_back()");
    }

    void pop_back()
    {
        if(m_type == T_LIST)
        {
            auto &list = Value<list_t>();
            list.pop_back();
            return;
        }
        throw std::logic_error("not a list type! JObject::pop_back()");
    }

    JObject &operator[](string const key)
    {
        if(m_type == T_DICT)
        {
            auto &dict =Value<dict_t>();
            return dict[key];
        }
        throw std::logic_error("not a dict type! JObject::operator[]()");
    }
    
    private:
        // 根据类型获取值的地址，直接硬转为void*类型，然后外界调用Value函数进行类型的强转
        void *value();
        
    private:
        TYPE m_type;
        value_t m_value;
    };
}






#endif