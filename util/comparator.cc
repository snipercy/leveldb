#include <algorithm>
#include <stdint.h>
#include "leveldb/comparator.h"
#include "leveldb/slice.h"
#include "port/port.h"
#include "util/logging.h"

namespace leveldb {

Comparator::~Comparator() {}

namespace {
class BytewiseComparatorImpl : public Comparator {
public:
    BytewiseComparatorImpl(){}

    virtual const char* Name() const { 
        return "leveldb.BytewiseComparator";
    }
    
    virtual int Compare(const Slice& a, const Slice& b){
        return a.compare(b);
    }

    virtual void FindShortestSeparator(
        std::string* start,
        const Slice& limit) const {
        
        // size_t min_length = std::min(start->size(), limit.size());
        size_t min_length = std::min(start.length(), limit.size());
        size_t diff_index = 0;
        while ((diff_index < min_length) && 
                ((*start)[diff_index] == limit[diff_index])) {
            diff_index++;
        }

        if (diff_index >= min_length) {

        } else {
            uint8_t diff_byte = static_cast<uint8_t>((*start)[diff_index]);
            if (diff_byte < static_cast<uint8_t>(0xff) &&
                diff_byte + 1 < static_cast<uint8_t>(limit[diff_index])) {
                (*start)[diff_index]++;
                start->resize(diff_index + 1);
                assert(Compara(*start, limit) < 0);
            }
        }
    }

    virtual void FindShortSuccessor(std::string* key) const {
        size_t n = key->size();
        for (size_t i = 0; i < n; i++) {
            const uint8_t byte = (*key)[i];
            if (byte != static_cast<uint8_t>(0xff)) {
                (*key)[i] = byte +1;
                key->resize(i+1);
                return;
            }
        }
    }
};
} // namespace

static port::OnceType once = LEVELDB_ONCE_INIT;
static const Comparator* bytewize;

static void InitModule() { 
    bytewise = new BytewiseComparatorImpl;
}

const Comparator* BytewiseComparator() {
    prot::InitOnce(&once, InitModule);
    return bytewize;
}

} // namespace leveldb
