/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file kv_iter.h
 * @author qb.wu@outlook.com
 * @date 2020/06/20
 * @brief
 **/

#ifndef PERSONAL_CODE_TOP_PV_KV_ITER_H
#define PERSONAL_CODE_TOP_PV_KV_ITER_H

#include <iostream>
#include <type_traits>
#include <iterator>

namespace toppv {

template <typename KeyIter, typename KeyGetter, typename ValueGetter>
class KVIter {
public:
    using iterator_type = KeyIter;
    using key_type = typename KeyGetter::type;
    using value_type = typename ValueGetter::type;
    using entry_type = std::pair<key_type, value_type>;

    explicit KVIter(KeyIter iter) : _iter(std::move(iter)) {}
    KVIter() = default;

    KVIter(KVIter &&) = default;
    KVIter& operator=(KVIter &&) = default;

    key_type key() const { return _get_key(*_iter); }
    value_type value() const { return _get_value(*_iter); }

    entry_type operator*() const {
        return std::make_pair(key(), value());
    }

    KVIter& operator++() {
        ++_iter;
        return *this;
    }

    bool operator==(const KVIter &rhs) const {
        return _iter == rhs._iter;
    }

    bool operator!=(const KVIter &rhs) const {
        return !(*this == rhs);
    }

private:
    KeyIter _iter;

    KeyGetter _get_key;
    ValueGetter _get_value;
};

template <typename Data>
struct Identity {
    using type = Data;
    const type& operator()(const type &data) const { return data; }
};

template <size_t Inc = 1>
struct Increment {
    using type = size_t;

    template <typename Data>
    const type& operator()(const Data &/*data*/) const {
        static const type ret = Inc;
        return ret;
    }
};

} // toppv

#endif // PERSONAL_CODE_TOP_PV_KV_ITER_H
