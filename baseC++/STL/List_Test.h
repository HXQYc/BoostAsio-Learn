#include "List.h"


static int testList()
{
    // 创建一个空的整数list
    List<int> numbers;

    // 向list末尾添加元素
    numbers.push_back(100);
    numbers.push_back(200);
    numbers.push_back(300);

    // 向list前端添加元素
    numbers.push_front(50);

    // 遍历list
    std::cout << "所有元素: ";
    for (auto it = numbers.begin(); it != numbers.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // 插入元素
    auto it = numbers.begin();
    ++it; // 指向第二个元素
    numbers.insert(it, 150);

    // 打印插入后的list
    std::cout << "插入元素后: ";
    for (auto num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // 删除元素
    numbers.remove(200);

    // 打印删除后的list
    std::cout << "删除元素后: ";
    for (auto num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}