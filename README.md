# 纸上得来终觉浅，绝知此事要躬行
本项目基本就是重新敲一遍源码，加上一些注释和修改某些函数的实现方式，功能保持不变，目的就是学习大神们如何设计及编写出优秀的代码。

---

## 初试牛刀
1. 编译源码，只需在包含makefile文件的目录下执行make命令即可，很顺利编译完成。
2. 写一个程序(`test.cc`)使用该库，代码如下：

```c++
#include <include/leveldb/db.h>
#include <string.h>
#include <assert.h>
#include <iostream>
int main(){
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;cy_test: test.o $(LIBOBJECTS) $(TESTHARNESS)
    leveldb::Status status = leveldb::DB::Open(options,"/tmp/test", &db);
    assert(status.ok());

    std::string key="vic";
    std::string value = "666";

    status = db->Put(leveldb::WriteOptions(), key,value);
    assert(status.ok());

    status = db->Get(leveldb::ReadOptions(), key, &value);
    assert(status.ok());
    std::cout << key <<"'s value is:" << value << std::endl;
    delete db;
    return 0;
}
```
3. 编译该测试程序，我觉得最简单的方法就是修改原makefile文件，新增一个目标对象即可。例如，新增cy_test目标对象:

```
cy_test: test.o $(LIBOBJECTS) $(TESTHARNESS)
    $(CXX) $(LDFLAGS) test.o $(LIBOBJECTS) $(TESTHARNESS) -o $@ $(LIBS)
```
输入`make cy_test`后运行`./cy_test`得到结果：`vic's value is:666`

------
# 下面记录一些值得注意的地方

#### 1. data()和c_str()

> 作者在Slice中的构造函数上使用string的成员函数`data()`将`string`转换成`char *`：
```
class Slice {
 public:
  ...
  // Create a slice that refers to the contents of "s"
  Slice(const std::string& s) : data_(s.data()), size_(s.size()) { }
  ...
```

c++98标准中关于data()有这样一段话：
> Accessing the value at data()+size() produces undefined behavior: There are no guarantees that a null character terminates the character sequence pointed by the value returned by this function.

通俗点说，data()转换后的字符数组结尾不保证有终结符`'\0'`，标准只是没有要求一定要有`'\0'`，不代表一定没有`'\0'`，不同的编译器可能得的结果不同，准确地说，这里的不同的编译器其实是指不同的`C++ STL`版本。

不过，这一问题在C++11标准中得到了解决，data()转换后的字符数组以'\0'结尾，同c_str()一样，原文描述如下：
> 
Returns a pointer to an array that contains a null-terminated sequence of characters (i.e., a C-string) representing the current value of the string object.
This array includes the same sequence of characters that make up the value of the string object plus an additional terminating null-character ('\0') at the end.

#### 2.


