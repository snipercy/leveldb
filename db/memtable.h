#ifndef STORAGE_LEVELDB_DB_MEMTABLE_H_
#define STORAGE_LEVELDB_DB_MEMTABLE_H_

#include <string>
#include "leveldb/db.h"
#include "db/dbformat.h"
#include "db/skiplist.h"
#include "util/arena.h"

namespace leveldb {

class InternalKeyComparator;
class Mutex;
class MemTableIterator;

class MemTable {
public:
    // MemTables are reference counted. The initial reference count
    // is zero and the caller must call Ref() at least once.
    explicit MemTable(const InternalKeyComparator& comparator);

    void Ref() { ++ refs_; }

    void Unref() {
        --refs_;
        assert(refs_ >= 0);
        if (refs_ <= 0) {
            delete this;
        }
    }

    size_t ApproximateMemoryUsage();

    // 该类只是作为一个接口，成员函数最终都是调用了skiplist类中的函数

    // Return an iterator that yields the contents of the memtable.
    //
    // The caller must ensure that the underlying MemTable remains live
    // while the returned iterator is live.  The keys returned by this
    // iterator are internal keys encoded by AppendInternalKey in the
    // db/format.{h,cc} module.
    Iterator* NewIterator();

    // Add an entry into memtable that maps key to value at the
    // specified sequence number and with the specified type.
    // Typically value will be empty if type==kTypeDeletion.
    void Add(SequenceNumber seq, ValueType type,
             const Slice& key,
             const Slice& value);

    // If memtable contains a value for key, store it in *value and return true.
    // If memtable contains a deletion for key, store a NotFound() error
    // in *status and return true.
    // Else, return false.
    bool Get(const LookupKey& key, std::string* value, Status* s);

private:
    ~MemTable();

    struct KeyComparator {
        const InternalKeyComparator comparator;
        explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) { }
        int operator()(const char* a, const char* b) const;
    };

    friend class MemTableIterator;
    friend class MemTableBackwardIterator;

    typedef SkipList<const char*, KeyComparator> Table;

    KeyComparator comparator_;
    int refs_;
    Arena arena_;
    Table table_;

    // no copying aollowed
    MemTable(const MemTable&);
    void operator=(const MemTable&);
};

} // namespace leveldb
#endif
