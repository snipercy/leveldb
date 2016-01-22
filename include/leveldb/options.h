// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_INCLUDE_OPTIONS_H_
#define STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

#include <stddef.h>
#include "filter_policy.h"

namespace leveldb {
    //class FilterPolicy;
    class Cache;
    class Comparator;
    class Env;
    class Logger;
    class Snapshot;

// DB contents are stored in a set of blocks, each of which holds a
// sequence of key,value pairs.  Each block may be compressed before
// being stored in a file.  The following enum describes which
// compression method (if any) is used to compress a block.
// 压缩存储
enum CompressionType {
    kNoCompression      = 0x0,
    kSnappyCompression  = 0x1
};

struct Options {
    const Comparator* comparator;

    // If true, the database will be created if it is missing.
    // Default: false
    bool create_if_missing;

    // If true, an error is raised if the database already exits.
    // Default: false
    bool error_if_exists;

    // If true, the implementation will do aggressive checking of the
    // data it is processing and will stop early if it detects any
    // errors.  This may have unforeseen ramifications: for example, a
    // corruption of one DB entry may cause a large number of entries to
    // become unreadable or for the entire DB to become unopenable.
    // Default: false
    bool paranoid_checks;

    Env* env;

    Logger* info_log;

    // default: 4MB
    size_t write_buffer_size;

    // Number of open files that can used by DB. You may need to
    // increase this if your databases has a large working set (budget 
    // one open file per 2MB of working set).
    //
    // Default: 1000
    int max_open_files;

    // Control over blocks (user data is stored in a set of blocks, and
    // a block is the unit of reading from disk).
    //
    // If non-NULL, use the specified cache for blocks.
    // If NULL, leveldb will automatically create and use an 8MB internal cache.
    // Default: NULL
    Cache* block_cache;


    // Approximate size of user data packed per block.  Note that the
    // block size specified here corresponds to uncompressed data.  The
    // actual size of the unit read from disk may be smaller if
    // compression is enabled.  This parameter can be changed dynamically.
    //
    // Default: 4K
    size_t block_size;

    // Number of keys between restart points for delta encoding of keys.
    // This parameter can be changed dynamically.  Most clients should
    // leave this parameter alone.
    //
    // Default: 16
    int block_restart_interval;

    // Compress blocks using the specified compressino algorithm. This
    // parameter can be changed dynamically.
    //
    // Default: kSnappyCompression, which give lightweight but fast 
    // compression.
    //
    // Typical speeds of kSnappyCompression on an Inter(R) Core(TM)2 2.4GHz:
    //  ~200-500MB/s compression
    //  ~400-800MB/s decompression
    // Note that these speeds are significantly faster than most persistent
    // storage speeds,  and therefore it is typically never worth switching
    // to kNoCompression. Even if the input data is imcompressible,the
    // kSnappyCompression implementation will efficiently detect that and 
    // will switch to uncompressed mode.
    CompressionType compression;

    // EXPRIMENTAL: If true,append to exiting MANIFEST and log files 
    // when a database is opened. This can significantly speed up open.
    //
    // Default: currently false, but may become true later.
    bool reuse_logs;

    // if non-NULL, user the specified filter to reduce disk reads.
    // Many applications will benifit from passing the result of
    // NewBloomFilterPolicy() here.
    //
    // Default: NULL
    const FilterPolicy* filter_policy;

    //Create an Option object with default values for all fields.
    Options();
}; // struct Options

struct ReadOptions {
    // 是否对督导的数据做校验
    bool verify_checksums;

    // should the data read for this iteration be cached in memory?
    // Callers may whith to set this field to false for bulk scans.
    // Default: true
    bool fill_cache;

    // If "snapshot" is non-NULL, read as of the supplied snapshot 
    // (which must belong to the DB that is being read and which must
    // not have been released). If "snapshot" is NULL, use an implicit
    // snapshot of the state at the beginning of this read operation.
    // 
    // Default: NULL
    // 指定读取的snapshot
    const Snapshot* snapshot;

    ReadOptions()
        : verify_checksums(false),
          fill_cache(true),
          snapshot(NULL){ }
}; // struct ReadOptions

struct WriteOptions {
    // If true, the write will be flushed from the operating system
    // buffer cache (by calling WritableFile::Sync()) before the write
    // is considered complete. If this flag is true, writes will be 
    // slower.
    //
    // If this is false, and the machine crashed, some recent 
    // writes may be lost. Note that if it is just the process that
    // crashes(i.e., the machine does not reboot), no writes will be
    // lost even if sync == false.
    // 
    // Default: false
    // 是否及时将内存中数据写入磁盘
    bool sync;

    WriteOptions()
        : sync(false) { }
};

} //namespace leveldb

#endif // STORAGE_LEVELDB_INCLUDE_OPTIONS_H_

