/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file mem_allocator.cpp
 * @author qb.wu@outlook.com
 * @date 2020/06/19
 * @brief
 **/

#include "mem_allocator.h"

#include <iostream>

namespace {

const size_t BLOCK_SIZE = 1 << 12;

}

namespace toppv {

MemAllocator::MemAllocator(size_t block_sz) {
    assert(block_sz > 0);
    auto mod = block_sz & (BLOCK_SIZE - 1);
    _block_sz = block_sz + (mod > 0 ? (BLOCK_SIZE - mod) : 0);
    std::cerr << "Use block size: " << _block_sz << "\n";
}

char* MemAllocator::_alloc_fallback(size_t bytes) {
    assert(bytes > 0);
    if (bytes > _block_sz / 4) {
        return _alloc_new_block(bytes);
    }
    _alloc_ptr = _alloc_new_block(_block_sz);
    _left_bytes = _block_sz;
    return _alloc_inplace(bytes);
}

char* MemAllocator::_alloc_new_block(size_t block_bytes) {
  char* result = new char[block_bytes];
  _blocks.push_back(result);
  _mem_usage += sizeof(char*) + block_bytes;
  return result;
}
} // toppv
