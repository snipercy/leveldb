# 纸上得来终觉浅，绝知此事要躬行
本项目基本就是重新敲一遍源码，加上一些注释和修改某些函数的实现方式，功能保持不变，目的就是学习大神们如何设计及编写出优秀的代码。

---

## 初试牛刀
> 据leveldb的设计可知，leveldb适合`写`操作多于`读`操作的应用场合，即写的效率高于读的效率，顺序读取的效率高于随机读取的效率。

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

#### 1. include/leveldb/slice.h  data()和c_str()

作者在Slice中的构造函数上使用string的成员函数`data()`将`string`转换成`char *`：
>
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

-----

#### 2. include/leveldb/options.h

在`options.h`文件中，作用通过声明的方式来使用其他文件中对应的类，如下所示：
```c++
#include <stddef.h>
namespace leveldb {
    class FilterPolicy;
    class Cache;
```
我修改如下，显示地引入头文件，注释了类的声明。
```c++
#include <stddef.h>
#include "filter_policy.h"
namespace leveldb {
    // class FilterPolicy;
    class Cache;
```

上述的做法本质是相同的，通过宏指令`#include`显示地把需要的头文件包含进来，这些头文件会在`预处理阶段`被编译器替换成相应的代码。

若不使用`#include`，只需显示地给出需要用到的类声明即可。 那么，预处理阶段没有找到类的实现代码，但它不会报错，它默认认为这该类在其他文件中给出了定义，此处会增加标记告诉编译器此符号未定义，需要在`链接阶段`找到它的定义，从哪找？从参与链接`目标文件`中取找，怎么找？遍历目标文件的符号表(Linux中目标文件的格式是`ELF`，符号表是ELF文件中的一个`entry`)。
>
[挖坑:]()整理一下编译器的工作过程->链接的过程->ELF文件的格式(2种，可链接可执行的和可执行的)->ELF格式为何要这样设计?(联系虚存的中各种area，方便加载到虚存中)->虚拟内存相关知识。)

最终编译出的库是完全一样的，我觉得各有千秋：
- `#include`的方法缩短编译时间，避免了`链接阶段`的遍历；
- 而通过声明的方式是减少了编程人员的工作，避免了头文件重复引用的问题。

#### 3. include/leveldb/status.h util/status.cc

在 `status.h`中，有一些成员函数，如：
```c++
// Return error status of an appropriate type.
  static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kNotFound, msg, msg2);
  }
```
调用了自定义的有参构造函数，该构造函数的定义如下：
```c
...
Status(Code code, const Slice& msg, const Slice& msg2) {
  assert(code != kOk);
  const uint32_t len1 = msg.size();
  const uint32_t len2 = msg2.size();
  const uint32_t size = len1 + (len2 ? (2 + len2) : 0);
  char* result = new char[size + 5];
  memcpy(result, &size, sizeof(size));
  result[4] = static_cast<char>(code);
  memcpy(result + 5, msg.data(), len1);
  if (len2) {
    result[5 + len1] = ':';
    result[6 + len1] = ' ';
    memcpy(result + 7 + len1, msg2.data(), len2);
  }
  state_ = result;
}
...
```
在msg是用户自定义的错误描述字符串，用法如下：
```c++
...
return Status::NotFound("in-memory file skipped past end");
...
```
看到这里，有一点不太明白: 既然`msg`是自定义的错误描述信息，那么`msg2`的存在意义是什么呢？

#### 4. include/leveldb/comparator.h  util/comparator.cc

对key排序时使用的比较方法。默认为升序。

#### 5. include/leveldb/write_batch.h  db/write_batch.cc

对若干数目 key 的 write 操作(put/delete)封装成 WriteBatch

#### 6. util/coding.h  util/coding.cc

为了节省空间，原作者设计了一种变长编码方式来表示整型：varint。越小的数字所用的字节数越少。
##### Varint编码
一般int需要3-byte来表示一个整数，varint使用的变长编码的方式和utf-8编码的方法本质上是一样的，下面表示的一个3字节的utf8编码：
''' 1110xxxx 10xxxxxx 10xxxxxx ```
第一字节中的从最高位开始，有连续几个`1`就表示该字符有几个字节，后面的每一字节都以`10`开头。

同样的道理，在varint变长编码中，如果每个字节的最高为1，表示后续的字节也是该数字的一部分；如果该位位0，则结束。
```c++
// 变长编码
//
// 每个数字用1到5个字节来编码，每个字节中的最高bit为标识位，
// 剩下的7位用于表示数据
char* EncodeVarint32(char* dst, uint32_t v) {
  // Operate on characters as unsigneds
  unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
  static const int B = 128;
  if (v < (1<<7)) {             // iff v < 128 then encode with 1-byte
    *(ptr++) = v;
  } else if (v < (1<<14)) {     // iff 128 <= v < 2^14 then ... 2-byte
    *(ptr++) = v | B;
    *(ptr++) = v>>7;
  } else if (v < (1<<21)) {
    *(ptr++) = v | B;
    *(ptr++) = (v>>7) | B;
    *(ptr++) = v>>14;
  } else if (v < (1<<28)) {
    *(ptr++) = v | B;
    *(ptr++) = (v>>7) | B;
    *(ptr++) = (v>>14) | B;
    *(ptr++) = v>>21;
  } else {                      // encode with with 5-byte
    *(ptr++) = v | B;
    *(ptr++) = (v>>7) | B;
    *(ptr++) = (v>>14) | B;
    *(ptr++) = (v>>21) | B;
    *(ptr++) = v>>28;
  }
  return reinterpret_cast<char*>(ptr);
}
void PutVarint32(std::string* dst, uint32_t v) {
  char buf[5];
```

##### varint解码
```c++
inline const char* GetVarint32Ptr(const char* p,
                                  const char* limit,
                                  uint32_t* value) {
  if (p < limit) {
    // 非const转换成const
    uint32_t result = *(reinterpret_cast<const unsigned char*>(p));
    // 只有一个字节
    if ((result & 128) == 0) {
      *value = result;
      return p + 1;
    }
  }

  // 多字节解码
  return GetVarint32PtrFallback(p, limit, value);
}
```
多字节解码函数
```c++
const char* GetVarint32PtrFallback(const char* p,
                                   const char* limit,
                                   uint32_t* value) {
  uint32_t result = 0;
  for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
    uint32_t byte = *(reinterpret_cast<const unsigned char*>(p));
    p++;
    if (byte & 128) {    // 最高位位1，还有后续字节
      // More bytes are present
      result |= ((byte & 127) << shift);
    } else {
      result |= (byte << shift);
      *value = result;
      return reinterpret_cast<const char*>(p);
    }
  }
  return NULL;
}
```

#### 6.SkipList

当我们插入的记录一条记录时，会先写log日志，然后将数据插入到memtable中，从名字也可以看出，memtable是在内存中的，当数据量到达一定量后再写到硬盘中，用sstable数据结构来组织数据的。

memtable是利用有名的`SkipList`来组织数据的，它是由William Pugh在论文：Skip lists: a probabilistic alternative to balanced trees中提出。下面的gif很形象的展示了skiplist的结构和查询/插入的过程，每个节点的level是随机生成的，动图中的`coin flip`展示了这一过程。一图胜千言。图片摘自[wiki] (https://en.wikipedia.org/wiki/Skip_list)
![skiplist](https://upload.wikimedia.org/wikipedia/commons/2/2c/Skip_list_add_element-en.gif)

关于它的分析，网上有很多的类似博文，可以参考[segmentfault中的这篇] (http://segmentfault.com/a/1190000003051117)
