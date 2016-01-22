// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A Status encapsulates the result of an operation.  It may indicate success,
// or it may indicate an error with an associated error message.
//
// Multiple threads can invoke const methods on a Status without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Status must use
// external synchronization.

#ifndef STORAGE_LEVELDB_INCLUDE_STATUS_H_
#define STORAGE_LEVELDB_INCLUDE_STATUS_H_

#include <string>
#include "leveldb/slice.h"

namespace leveldb {
class Status {
public:
    // create a success status
    Status() : state_(NULL){ }
    ~Status() { delete[] state_; }

    // copy the spcified status.
    // 拷贝构造函数和重装赋值运算符
    Status(const Status& s);
    void operator=(const Status& s);
    // return a success status.
    static Status OK() { return Status(); }

    // return error status of an appropriate type.
    //
    // i.e., return Status::NotFound("in-memory file skipped past end");
    // msg 自定义错误描述
    static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) { 
        return Status(kNotFound, msg, msg2);
    }
    static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) { 
        return Status(kCorruption, msg, msg2);
    }
    static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) { 
        return Status(kNotSupported, msg, msg2);
    }
    static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) { 
        return Status(kInvalidArgument, msg, msg2);
    }
    static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) { 
        return Status(kIOError, msg, msg2);
    }

    // returns true if the status indicates success.
    bool ok() const { return (state_ == NULL); }

    // returns true if the status indicates a NotFound error.
    bool IsNotFound() const { return code() == kNotFound; }

    // returns true if the status indicates a corruption error.
    bool IsCorruption() const { return code() == kCorruption; }

    bool IsIOError() const { return code() == kIOError; }

    bool IsNotSupportedError() const { return code() == kNotSupported; }

    bool IsInvalidArgument() const { return code() == kInvalidArgument; }

    // Return a string representation of this status suitable for printing.
    // returns the string "OK" for success.
    std::string ToString() const;

private:
    // OK status has a NULL state_. Otherwise, state_ is a new[] array
    // of the following form:
    //      state_[0..3] == length of message
    //      state_[4]    == code
    //      state_[5..]  == message
    const char* state_;

    enum Code {
        kOk = 0,
        kNotFound = 1,
        kCorruption = 2,
        kNotSupported = 3,
        kInvalidArgument = 4,
        kIOError = 5
    };

    Code code() const {
        // static_cast<Code>能换成static_cast<int>吗？
        // hhhhh 
        return (state_ == NULL) ? kOk : static_cast<Code>(state_[4]);
    }

    // 将2个信息拼接在一起
    Status(Code code, const Slice& msg, const Slice& msg2);
    static const char* CopyState(const char* s);
};

inline Status::Status(const Status& s) {
    state_ = (s.state_ == NULL) ? NULL : CopyState(s.state_);
}


inline void Status::operator=(const Status& s) {
    if (state_ != s.state_) {
        delete[] state_;
        state_ = (s.state_ == NULL) ? NULL : CopyState(s.state_);
    }
}

} // namespace leveldb
#endif
