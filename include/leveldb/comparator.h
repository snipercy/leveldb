#ifndef STORAGE_LEVELDB_INCLUDE_COMPARATOR_H_
#define STORAGE_LEVELDB_INCLUDE_COMPARATOR_H_

#include <string>

namespace leveldb {

class Slice;

class Comparator {
    public:
        virtual ~Comparator();
    
    virtual int Compare(const Slice& a, const Slice& b) const = 0;

    // The name of comparator.
    virtual const char* Name() const = 0;

    virtual void FindShortestSeparator(
            std::string* start,
            const Slice& limit) const = 0;

    virtual void FindShortSuccessor(std::string* key) const = 0;
};

extern const Comparator* BytewiseComparator();

} // namespace leveldb

#endif
