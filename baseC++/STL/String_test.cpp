#include "String_test.h"
#include <iostream>

void StringTest()
{
    // 1. 构造函数测试
    std::cout << "1. 构造函数" << std::endl;
    String s1;  // 默认构造
    String s2("Hello");  // C字符串构造
    String s3(s2);  // 拷贝构造
    std::cout << "  s1(默认): " << s1 << std::endl;
    std::cout << "  s2: " << s2 << std::endl;
    std::cout << "  s3: " << s3 << std::endl;

    // 2. 赋值运算符测试
    std::cout << "2. 赋值运算符" << std::endl;
    s1 = "World";
    std::cout << "  s1 = " << s1 << std::endl;

    String s4;
    s4 = s2;
    std::cout << "  s4 = s2: 拷贝赋值成功" << std::endl;
    std::cout << " s4 = " << s4 << std::endl;

    // 3. 拼接操作符 (+) 测试
    std::cout << "3.拼接操作符 (+) " << std::endl;
    s2 + " " + s1;  // Hello + " " + World
    std::cout << "  s2 + s1: 结果应为 'Hello World'" << std::endl;
    std::cout << s2 << std::endl;

    // 4. 查找功能测试
    std::cout << "4. 查找" << std::endl;
    String text("dog cat fox cow");
    int pos = text.find("cat");
    std::cout << "  查找 'cat' 位置: " << pos << std::endl;

    // 5. 删除操作符 (-) 测试
    std::cout << "5. 删除操作符 (-) " << std::endl;
    String s5("Hello World Hello");
    s5 - "Hello";
    std::cout << "  结果应为 ' World Hello'" << std::endl;
    std::cout << s5 << std::endl;

    // 6. 替换功能测试
    std::cout << "6. 替换" << std::endl;
    String s6("I like apples");
    bool replaceResult = s6.replace(7, "apples", "oranges");
    std::cout << "  替换 'apples' 为 'oranges': " << (replaceResult ? "成功" : "失败") << std::endl;

}