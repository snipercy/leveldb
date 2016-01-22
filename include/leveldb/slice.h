#ifndef STORAGE_LEVELDB_INCLUDE_SLICE_H_
#define STORAGE_LEVELDB_INCLUDE_SLICE_H_

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <string>

namespace leveldb {
class Slice {
public:
    // create an empyt slice
    Slice() : data_(""), size_(0) { }

    // create a slice that refers to d[0,n-1]
    Slice(const char* d, size_t n) : data_(d), size_(n) { }

    Slice(const std::string& s) : data_(s.data()), size_(s.size()){}

    Slice(const char* s) : data_(s), size_(strlen(s)) { }

    // return a pointer to the beginning of the referenced data
    const char* data() const { return data_; }

    // return the length (in bytes) of the referenced data
    size_t size() const { return size_; }

    // return true if the length of the referenced data is zero
    bool empty() const { return 0 == size_; }

    // return the ith byte in the referenced data.
    // REQUIRES: n < size()
    char operator[](size_t n) const {
        assert( n < size() );
        return data_[n];
    }

    // change this slice to refer to an empty array
    void clear() { data_ = ""; size_ = 0; }

    // drop the first "n" bytes from slices
    void remove_prefix(size_t n){
        assert( n <= size() );
        data_ += n;
        size_ -= n;
    }

    // return a string that contains the copy of the referenced data.
    // haha
    // inside in c++ model: NRV 优化
    std::string ToString() const { return std::string(data_, size_); }

    // Three-way comparison. Returns value:
    // <  0 iff *this <  b
    // == 0 iff *this == b
    // >  0 iff *this >  b
    int compare(const Slice& b) const;

    // return true iff 'x' is a prefix of *this
    bool starts_with(const Slice& x) const {
        return ( (size_ >= x.size_) &&
                 (memcmp(data_, x.data_, x.size_) == 0) );
    }

private:
    const char* data_;
    // size_t 增加程序的可移植性
    //  size_t is 64 bits on 64-bits architecture, 
    //  int remain 32bits long on those architectures
    //  To summarize : use size_t to represent the size of an object 
    //  and int (or long) in other cases.
    size_t size_;
};

inline bool operator==(const Slice& x, const Slice& y) { 
    return ((x.size() == y.size()) &&
            (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y){
    return !(x == y);
}

inline int Slice::compare(const Slice& b) const {
    const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
    int r = memcmp(data_, b.data_, min_len);
    if (r == 0){
        if (size_ < b.size_) r = -1;
        else if (size_ > b.size_) r = +1;
    }
    return r;
}

} // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_SLICE_H_
