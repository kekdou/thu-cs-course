## lambda 表达式

C++11 开始引入了 lambda 表达式，类似 JavaScript 的箭头函数，核心就是创建匿名函数对象

```C++
[capture list](parameter list) -> return type { function body}
```

- `capture list`: 捕获列表，指定 lambda 表达式可以访问的外部变量，以及按值或按引用的方式访问
- `parameter list`: 参数列表，和普通函数一样
- `return type`: 返回值类型，可以省略由编译器自己推导，也可以用 -> 显示指定
- `function body`: 函数体

需要注意的是捕获列表
- `[]`：不捕获任何变量
- `[=]`：按值捕获所有外部变量
- `[&]`：按引用捕获所有外部变量
- `[x, &y]`：特定捕获 x 按值， y 按引用

```C++
int factor = 10;
auto multiply = [factor](int n) -> int {return n * factor;};
factor = 20;
cout << multiply(10) << endl;
// 输出 100
```

对于按值捕获，会在表达式定义时就捕获，factor 不会再随外部变化而变化，按引用捕获则不会，在表达式调用时再捕获，且内部可修改  

lambda 表达式经常配合 algorithm 使用，在 C++14后大加强，可以用 auto 实现泛型

```C++
auto add = [](auto a, auto b) {
    return a + b;
}; 
```

值得注意的是，我们使用 `auto multiply` 和 `auto add` 是因为 lambda 返回一个闭包对象

```C++
// 编译器生成的“隐形”类
class __unique_lambda_name {
public:
    // 对应 Lambda 的函数体
    int operator()(int a, int b) const {
        return a + b;
    }
};
// add 实际上是这个类的一个对象
__unique_lambda_name add = __unique_lambda_name();
```

这个类的名字是编译器随机生成的（如 `__lambda_6a2b...`），无法写出名字，因此常使用 `auto`，当然也有别的方法

## 自定义函数

在编写程序的过程中，一种算法（如排序）本身是固定的，但它执行时采用的策略（如比较逻辑）是可以替换的，在 C++ 范式中称为“策略模式”  

其中可以分为：函数模板(如 std::sort)接受一个可调用对象(函数指针，lambda表达式或函数对象)，以及类模板(如 std::priority_queue)接受一个类型(结构体或类)


### 函数模板(以 sort 为例) 

```C++
sort(container.begin(), container.end(), compare_function)
```

在默认情况下，编译器使用 `std::less<T>` 底层逻辑等同于使用元素类型的 `<` 运算符，按照严格弱序排列，直观理解就是若 comp(a, b) 为 true，则 a 排在 b 前面
- **数值类型**：按数值大小排序(如 $1 < 2 < 3$)
- **字符/字符串**：按字典序(ASCII 码顺序)排序(如 "Apple" < "Banana")
- **自定义结构体**：如果没有为结构体重载 < 运算符，会编译错误

修改比较函数的实现方式

1. 使用 lambda 表达式

```C++
sort(nums.begin(), nums.end(), [](int& a, int& b) {return a > b;});
```

2. 自定义函数

```C++
bool myCompare(int& a, int& b) {
    return a > b;
}
sort(nums.begin(), nums.end(), myCompare);
```

对于自定义类型，如结构体和类也同样的方式，当然也可以在自定义类型内部或全局重载 `<` 


内部重载：

```C++
struct Student {
    int id;
    int score;
    std::string name;
    bool operator<(const Student& other) const {
        if (score != other.score) {
            return score > other.score;
        }
        return id < other.id;
    }
};
sort(stu.begin(), stu.end()) // 实现按分数降序，分数相同按id升序排列
```

全局重载：

```C++
struct Point {
    int x, y;
};
bool operator<(const Point& a, const Point& b) {
    return a.x < b.x;
}
sort(p.begin(), p.end()) // 按 x 坐标升序
```

### 类模板

```C++
std::priority_queue<Type, Container, Functional>
```

默认情况下使用 std::less，最大堆，若 comp(a, b) 为 true，则 b 的优先级更高（排在前面）

与 sort 不同注意区分

修改逻辑实现方式：

1. 使用内置仿函数

如果只需简单的升序/降序，使用 `functional` 库

```C++
// 声明一个最小堆
priority_queue<int, vector<int>, greater<int>> pq;
```

2. 自定义仿函数

```C++
struct MyCompare {
    bool operator()(int a, int b) {
        return a > b // 牢记返回 ture, 说明 a < b
    }
}
// 声明一个最小堆
priority_queue<int, vector<int>, MyCompare> pq;
```

关于仿函数还有很多内容...