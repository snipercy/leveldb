
#ifndef STORAGE_LEVELDB_DB_SKIPLIST_H_
#define STORAGE_LEVELDB_DB_SKIPLIST_H_

// Thread safety
// -------------
// 
// Writes require external synchronization, most likely a mutex.
// Reads require a guarantee that the SkipList will not be destroyed
// while the read is in progress.  Apart from that, reads progress
// without any internal locking or synchronization.
// 
// Invariants:
// 
// (1) Allocated nodes are never deleted until the SkipList is
// destroyed.  This is trivially guaranteed by the code since we
// never delete any skip list nodes.
// 
// (2) The contents of a Node except for the next/prev pointers are
// immutable after the Node has been linked into the SkipList.
// Only Insert() modifies the list, and it is careful to initialize
// a node and use release-stores to publish the nodes in one or
// more lists.
// 
// ... prev vs. next pointer ordering ...

#include <assert.h>
#include <stdlib.h>
#include "port/port.h"
#include "util/arena.h"
#include "util/random.h"

namespace leveldb {

class Arena;

template<typename Key, class Comparator>
class SkipList {
private:
    struct Node;

public:
    // Create a new SkipList object that will use "cmp" for comparing keys,
    // and will allocate memory using "*arena".  Objects allocated in the arena
    // must remain allocated for the lifetime of the skiplist object.""""
    // 构造函数，cmp指定比较算子，并分配内存
    explicit SkipList(Comparator cmp, Arena* arena);

    void Insert(const Key& key);

}

// constructor
template<typename Key, class Comparator>
SkipList<Key, Comparator>::SkipList(Comparator cmp, Arena* arena)
    : compare_(cmp),
      arena_(arena),
      head_(NewNode(0, kMaxHeight)),
      rnd_(oxdeadbeef) {
          for (int i = 0; i< kMaxHeight; i++) { 
            head_->SetNext(i, NULL) ;
          }
      }

} // namespace leveldb

#endif
