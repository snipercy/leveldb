// WriteBatch唯一成员string rep_格式
//      sequence: 64bit
//      count:    32bit   有多少条记录需要处理
//      data:     recode[count]  具体某条记录
//    recode :=
//          (kTypeValue 为记录类型，更新or删除,
//           删除时只有key) )
//          kTypeValue varstring varstring. kTypeValue |
//          kTypeValue varstring(key)
//    varstring :=
//          len: varint32
//          data: uint8[len]

#include "leveldb/write_batch.h"

#include "leveldb/db.h"
#include "db/dbformat.h"
#include "db/memtable.h"
#include "db/write_batch_internal.h"
#include "util/coding.h"

namespace leveldb {

// sequence(8-byte) and count(4-byte)
static const size_t kHeader  = 12;

WriteBatch::WriteBatch() {
    Clear();
}

WriteBatch::~WriteBatch() {}

WriteBatch::Handler::~Handler(){}

void WriteBatch::Clear() {
    rep_.clear();
    rep_.resize(kHeader);
}

Status WriteBatch::Iterate(Handler* handler) const {
    Slice input(rep_);
    if (input.size() < kHeader) {
        return Status::Corruption("malformed WriteBatch(too small)");
    }

    // 移除req_ 的seq num和count
    input.remove_prefix(kHeader);
    Slice key, value;
    int found = 0;
    while (!input.empty()) {
        found++;
        char tag = input[0];
        input.remove_prefix(1);
        switch (tag) {
            case kTypeValue:
               if (GetLengthPrefixedSlice(&input, &key) &&   //get Key
                  GetLengthPrefixedSlice(&input, &value)) {  //get value
                    handler->Put(key, value);
               } else {
                    return Status::Corruption("bad WriteBatch Put");
               }
               break;
               
            case kTypeDeletion:
               if (GetLengthPrefixedSlice(&input, &key)) {
                    handler->Delete(key);
               } else {
                    return Status::Corruption("bad WriteBatch Delete");
               }
               break;
             default:
               return Status::Corruption("unknown WriteBatch tag");
        }
    } // while
        
    if (found != WriteBatchInternal::Count(this)) {
        return Status::Corruption("WriteBatch has wrong count");
    } else {
        return Status::OK(); 
    }
}

int WriteBatchInternal::Count(const WriteBatch* b) {
    return DecodeFixed32(b->rep_.data() + 8);
}

// 设置读取记录数
void WriteBatchInternal::SetCount(WriteBatch* b, int n) {
    EncodeFixed32(&b->rep_[8], n);
}

SequenceNumber WriteBatchInternal::Sequence(const WriteBatch* b) {
    return SequenceNumber(DecodeFixed64(b->rep_.data()));
}

void WriteBatchInternal::SetSequence(WriteBatch* b, SequenceNumber seq) {
  EncodeFixed64(&b->rep_[0], seq);
}

void WriteBatch::Put(const Slice& key, const Slice& value) {
    WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
    rep_.push_back(static_cast<char>(kTypeValue));

    // 将key append到rep_后面
    PutLengthPrefixedSlice(&rep_, key);
    PutLengthPrefixedSlice(&rep_, value);
}

void WriteBatch::Delete(const Slice& key) {
    WriteBatchInternal::SetCount(this, WriteBatchInternal::Count(this) + 1);
    rep_.push_back(static_cast<char>(kTypeDeletion));
    PutLengthPrefixedSlice(&rep_, key);
}

namespace {
class MemTableInserter : public WriteBatch::Handler {
public:
    SequenceNumber sequence_;
    MemTable* mem_;

    virtual void Put(const Slice& key, const Slice& value) {
        mem_->Add(sequence_, kTypeValue, key ,value);
        sequence_++;
    }
    virtual void Delete(const Slice& key) {
        mem_->Add(sequence_, kTypeDeletion, key, Slice());
        sequence_++;
    }
};
}

Status WriteBatchInternal::InsertInto(const WriteBatch* b,
                                     MemTable* memtable) {
    MemTableInserter inserter;
    inserter.sequence_ = WriteBatchInternal::Sequence(b);
    inserter.mem_ = memtable;
    return b->Iterate(&inserter);
}

void WriteBatchInternal::SetContents(WriteBatch* b, const Slice& contents) {
    assert(contents.size() > kHeader);
    b->rep_.assign(contents.data(), contents.size());
}

void WriteBatchInternal::Append(WriteBatch* dst, const WriteBatch* src) {
    SetCount(dst, Count(dst) + Count(src));
    assert(src->rep_.size() > kHeader);
    dst->rep_.append(src->rep_.data() + kHeader, src->rep_.size() - kHeader);
}

} // namespace leveldb
