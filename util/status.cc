#include <stdio.h>
#include "port/port.h"
#include "leveldb/status.h"

#include <iostream>

namespace leveldb{

const char* Status::CopyState(const char* state) {
    // 大小为4字节
    uint32_t size;

    // 只拷贝state前4个字节，state前4字节为msg的长度
    memcpy(&size, state, sizeof(size));

    // msg的长度为size，5表示前4字节长度+errorno
    char* result = new char[size + 5];
    memcpy(result, state, size+5);
    return result;
}

// hhhhh 
// msg 是自定义描述信息，附加字符串; msg2有什么用？
// i.e., NotFound("in-memory file skipped past end");
//
// NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
//     status(kNotFound, msg, msg2);}

    assert(code != kOk);
    const uint32_t len1 = msg.size();
    const uint32_t len2 = msg2.size();
    const uint32_t size = len1 + (len2 ? (2 + len2) : 0);

    // for debug
    std::cout << "status.cc, len1:"<< len1 << " len2:" << len2
              <<" size:" << size << std::endl;

    char* result = new char[size + 5];
    memcpy(result, &size, sizeof(size));
    result[4] = static_cast<char>(code);
    memcpy(result+5, msg.data(), len1);
    if (len2) {
         result[5+ len1] = ':';
         result[6+ len1] = ' ';
         memcpy(result + 7 +len1, msg2.data(), len2);
    }
    state_ = result;

    // for debug
    std::cout << "result:" << result << endl;
}

std::string Status::ToString() const {
    if (state_ == NULL) {
        return "OK" ;
    } else {
        char tmp[30];
        const char* type;
        switch(code()) {
            case kOk:
                type = "OK";
                break;
            case kNotFound:
                type = "NotFound: ";
                break;
            case kCorruption:
                type = "Corruption: ";
                break;
            case kNotSupported:
                type = "Not implemented: ";
                break;
            case kInvalidArgument:
                type = "Invalid argument: ";
                break;
            case kIOError:
                type = "IO error: ";
                break;
            default:
                snpritf(tmp, sizeof(tmp),"Unknown code(%d): ",
                       static_cast<int>(code()));
                type = tmp;
                break;
        }
        std::string result(type);
        uint32_t length;
        memcpy(&length, state_, sizeof(length));

        // 截断原state_，只取后面的msg
        result.append(state_ + 5, length);
        return result;
    }
}

} // namespace leveddb
