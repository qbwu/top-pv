/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file mem_allocator.h
 * @author qb.wu@outlook.com
 * @date 2020/06/19
 * @brief
 **/

#ifndef PERSONAL_CODE_TOP_PV_MEM_ALLOCATOR_H
#define PERSONAL_CODE_TOP_PV_MEM_ALLOCATOR_H

#include <cassert>

#include <iostream>
#include <vector>

namespace toppv {

class MemAllocator {
public:
    explicit MemAllocator(size_t block_sz);

    MemAllocator(const MemAllocator&) = delete;
    MemAllocator& operator=(const MemAllocator&) = delete;

    ~MemAllocator() {
        clear();
    }

    char* alloc(size_t bytes) {
        if (bytes == 0) {
            return nullptr;
        }
        return bytes <= _left_bytes ?
            _alloc_inplace(bytes) : _alloc_fallback(bytes);
    }

    size_t mem_usage() const {
        return _mem_usage;
    }

    void clear() {
        for (auto *p : _blocks) {
            delete [] p;
        }
        _blocks.clear();
        _alloc_ptr = nullptr;

        _left_bytes = 0;
        _mem_usage = 0;
        _mem_fragment = 0;
        _mem_dirty = 0;
    }

    void log_profile() const {
        std::cerr << "mem_usage: " << _mem_usage
                  << ", mem_dirty: " << _mem_dirty
                  << ", mem_fregment: " << _mem_fragment
                  << ", block_size: " << _block_sz
                  << ", block_num: " << _blocks.size() << std::endl;
    }
private:
    char* _alloc_fallback(size_t bytes);
    char* _alloc_new_block(size_t block_bytes);

    char* _alloc_inplace(size_t bytes) {
        assert(bytes <= _left_bytes);
        auto *result = _alloc_ptr;
        _alloc_ptr += bytes;
        _left_bytes -= bytes;
        _mem_dirty += bytes;
        return result;
    }

    std::vector<char*> _blocks;
    char *_alloc_ptr = nullptr;

    size_t _left_bytes = 0;
    size_t _mem_usage = 0;
    size_t _mem_dirty = 0;
    size_t _mem_fragment = 0;
    size_t _block_sz = 0;
};

} // toppv

#endif // PERSONAL_CODE_TOP_PV_MEM_ALLOCATOR_H
