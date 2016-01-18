# 纸上得来终觉浅，绝知此事要躬行
本项目基本就是重新敲一遍源码，加上一些注释和修改某些函数的实现方式，功能保持不变，目的就是学习大神们如何设计及编写出优秀的代码。

---

## 初使牛刀
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
