#include <iostream>
#include "JObject.h"
#include "Parser.h"
#include <fstream>

using namespace json;

struct Base
{
    int pp;
    string qq;

    START_FROM_JSON
        pp = from("pp", int);
        qq = from("qq", string);
    END_FROM_JSON

    START_TO_JSON
        to("pp") = pp;
        to("qq") = qq;
    END_TO_JSON
};

struct Mytest
{
    int id;
    std::string name;
    Base q;

    START_TO_JSON
        to_struct("base", q);
        to("id") = id;
        to("name") = name;
    END_TO_JSON

    START_FROM_JSON
        id = from("id", int);
        name = from("name", string);
        from_struct("base", q);
    END_FROM_JSON
};

// void test_class_serialization()
// {
//     Mytest test{.id = 32, .name = "fda"};
//     auto item = Parser::FromJSON<Mytest>(R"({"base":{"pp":0,"qq":""},"id":32,"name":"fda"} )"); // serialization
//     std::cout << Parser::ToJSON(item);                                                          // deserialization
// }

void test_string_parser()
{
    std::ifstream fin(R"(../test_source/test.json)");
    if (!fin) {
        std::cout << "read file error";
        return;
    }
    // 提示：
    // std::istreambuf_iterator<char>(fin)：
    // 这是一个输入流迭代器，使用 fin 流（文件输入流）作为输入源，
    // char 表示它是字符流迭代器，每次迭代都会读取一个字符。

    // std::istreambuf_iterator<char>()：
    // 这是一个默认构造的、标记为结束的输入流迭代器，表示流的末尾。
    std::string text((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    {
        auto object = json::Parser::FromString(text);

        std::cout << (object["[markdown]"].to_string()) << "\n";
    }
}


enum T
{
    MYSD,
    sf
};

int main()
{
    test_string_parser();
}