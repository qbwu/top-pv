/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file mem_table.h
 * @author qb.wu@outlook.com
 * @date 2020/06/20
 * @brief
 **/

#ifndef PERSONAL_CODE_TOP_PV_MEM_TABLE_H
#define PERSONAL_CODE_TOP_PV_MEM_TABLE_H

#include <cstring>

#include <algorithm>
#include <vector>

#include "mem_allocator.h"

namespace toppv {

class MemTable {
public:
    explicit MemTable(size_t block_sz) : _allocator(block_sz) {}

    MemTable(const MemTable&) = delete;
    MemTable& operator=(const MemTable&) = delete;

    void insert(const char *data, size_t len) {
        auto *buf = _allocator.alloc(len + sizeof(Record));
        auto *rec = new (buf) Record{len, buf + sizeof(Record)};
        memcpy(const_cast<char*>(rec->data), data, len);
        _records.push_back(rec);
    }

    size_t size() const {
        return _records.size();
    }

    size_t mem_usage() const {
        return _allocator.mem_usage() + sizeof(Record*) * _records.capacity();
    }

    void sort() {
        std::sort(_records.begin(), _records.end(),
                [](Record *lhs, Record *rhs) { return *lhs < *rhs; });
    }

    bool empty() const {
        return _records.empty();
    }

    void clear() {
        _allocator.log_profile();
        _allocator.clear();
        _records.clear();
    }

    struct Record {
        const size_t len;
        const char *const data;

        bool operator<(const Record &rhs) const {
            auto res = memcmp(this->data, rhs.data, std::min(this->len, rhs.len));
            return res < 0 || (res == 0 && this->len < rhs.len);
        }

        bool operator==(const Record &rhs) const {
            return this->len == rhs.len
                && memcmp(this->data, rhs.data, this->len) == 0;
        }
    };

    using iterator_type = std::vector<Record*>::const_iterator;

    iterator_type begin() const {
        return _records.begin();
    }

    iterator_type end() const {
        return _records.end();
    }

private:
    MemAllocator _allocator;
    std::vector<Record*> _records;
};

} // toppv

#endif // PERSONAL_CODE_TOP_PV_MEM_TABLE_H
