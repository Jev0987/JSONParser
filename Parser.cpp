#include "Parser.h"
#include <cctype>
#include <algorithm>


using namespace json;

JObject Parser::FromString(string_view content)
{
    // 单例模式
    static Parser instance;
    instance.init(content);
    return instance.parse();
}

void Parser::init(std::string_view src)
{
    m_str = src;
    m_idx = 0;
    trim_right();  // 去除多余的空格
}

void Parser::trim_right()
{
    // 去除尾部空字符，方便最后的结束判断
    m_str.erase(std::find_if(m_str.rbegin(), m_str.rend(), [](char ch)
    {
        return !std::isspace(ch);
    }).base(), m_str.end());
}

void Parser::skip_comment()
{
    // 跳过注释
    if(m_str.compare(m_idx, 2, R"(//)") == 0)
    {
        while(true)
        {
            auto next_pos = m_str.find('\n', m_idx);
            if(next_pos == string::npos)
            {
                throw std::logic_error("invalid comment area!");
            }

            // 查看下一行是否有注释
            m_idx = next_pos + 1;
            while(isspace(m_str[m_idx]))
            {
                m_idx++;
            }

            if(m_str.compare(m_idx, 2, R"(//)") != 0)
            {
                // 注释结束
                return;
            }
        }
    }
}

char Parser::get_next_token()
{
    // 跳过空格和跳过注释
    while (std::isspace(m_str[m_idx])) m_idx++;
    if(m_idx >= m_str.size())
        throw std::logic_error("unexpected character in parse json");
    
    // 如果是注释，记得跳过
    skip_comment();
    return m_str[m_idx];
}

bool Parser::is_esc_consume(size_t pos)
{
    // 检查在pos处的反斜杠是否为转义字符。
    
    if(pos == 0 || m_str[pos] != '\\') return false;  // 如果是在字符串开头或者该位置不是反斜杠，不需要转义
    size_t end_pos = pos;
    while(m_str[pos] == '\\') pos--;
    auto cnt = end_pos - pos;
    // 如果 \ 的个数为偶数，则成功抵消，如果为奇数，则未抵消
    return cnt % 2 == 0;
}

JObject Parser::parse()
{
    char token = get_next_token();
    if(token == 'n')
    {
        return parse_null();
    }

    if(token == 't' || token == 'f')
    {
        return parse_bool();
    }
    if(token == '-' || std::isdigit(token))
    {
        return parse_number();
    }
    if(token == '\"')
    {
        return parse_string();
    }
    if(token == '[')
    {
        return parse_list();
    }
    if(token == '{')
    {
        return parse_dict();
    }
    throw std::logic_error("unexptected character in parse json");
}

JObject Parser::parse_null()
{
    
    if(m_str.compare(m_idx, 4, "null") == 0)
    {
        m_idx += 4;
        return {};
    }
    throw std::logic_error("parse null error");
}

JObject Parser::parse_number()
{
    auto pos = m_idx;
    // 负数符号
    if(m_str[m_idx] == '-')
    {
        m_idx++;
    }

    // 整数部分
    if(isdigit(m_str[m_idx]))
    {
        while(isdigit(m_str[m_idx]))
        {
            m_idx++;
        }
    }else{
        throw std::logic_error("invalid character in number");
    }

    // 小数部分
    if(m_str[m_idx] == '.')
    {
        m_idx++;
        if(!std::isdigit(m_str[m_idx]))
        {
            throw std::logic_error("at least one digit required in parse float part!");
        }
        while(std::isdigit(m_str[m_idx]))
        {
            m_idx++;
        }
    }
    
    // 提示：
    // strtod 将字符串m_str从pos位置开始的部分转换为双精度的浮点数。
    // 转换成功则返回双精度浮点数。
    return strtod(m_str.c_str() + pos, nullptr);
}

bool Parser::parse_bool()
{
    if(m_str.compare(m_idx, 4, "true") == 0)
    {
        m_idx += 4;
        return true;
    }
    if(m_str.compare(m_idx, 5, "false") == 0)
    {
        m_idx += 5;
        return false;
    }
    throw std::logic_error("parse bool error");
}

string Parser::parse_string()
{
    auto pre_pos = ++m_idx;
    auto pos = m_str.find('"', m_idx);

    // 提示：string::npos 是 std::string类中的一个静态常量，
    // string::npos 表示无效或不存在的位置
    if(pos != string::npos)
    {
        // 解析还没有结束，需要判断是否是转义的结束符号，如果是转义，则需要继续探查
        while(true)
        {
            if(m_str[pos - 1] != '\\')
            {
                break;
            }

            // 如果是转义字符，则判断是否已经被抵消，已经被抵消完则跳出，否则继续寻找下一个字符串的结束符号
            if(is_esc_consume(pos - 1))
            {
                break;
            }
            pos = m_str.find('"', pos + 1);
            if(pos == string::npos)
            {
                throw std::logic_error(R"(expected left '"' in parse string)");
            }
        }
        m_idx = pos + 1;
        return m_str.substr(pre_pos, pos - pre_pos);
    }
    throw std::logic_error("parse strng error");
}

JObject Parser::parse_list()
{
    JObject arr((list_t()));
    m_idx++;
    char ch = get_next_token();
    if(ch == ']')
    {
        m_idx++;
        return arr;
    }

    while(true)
    {
        arr.push_back(parse());
        ch = get_next_token();
        if(ch == ']')
        {
            m_idx++;
            break;
        }

        if(ch != ',')
        {
            throw std::logic_error("expected ',' in parse list");
        }
        // 跳过逗号
        m_idx++;
    }
    return arr;
}

JObject Parser::parse_dict()
{
    JObject dict((dict_t()));
    m_idx++;
    char ch = get_next_token();
    if(ch == '}')
    {
        m_idx++;
        return dict;
    }

    while(true)
    {
        // 解析key
        string key = std::move(parse().Value<string>());
        ch = get_next_token();

        if(ch != ':')
        {
            throw std::logic_error("expected ':' in parse dict");
        }
        m_idx++;

        // 解析value
        dict[key] = parse();
        ch = get_next_token();
        if(ch == '}')
        {
            m_idx++;
            break; // 解析完毕
        }
        if(ch != ',')
        {
            throw std::logic_error("expected ',' in parse dict");
        }
        // 跳过逗号
        m_idx++;
    }
    return dict;
}