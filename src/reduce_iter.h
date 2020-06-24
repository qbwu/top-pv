/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file reduce_iter.h
 * @author qb.wu@outlook.com
 * @date 2020/06/21
 * @brief
 **/

#ifndef PERSONAL_CODE_TOP_PV_REDUCE_ITER_H
#define PERSONAL_CODE_TOP_PV_REDUCE_ITER_H

#include <iostream>
#include <functional>

namespace toppv {

template <typename KVIter, typename V>
class ReduceIter {
public:
    using key_type = typename KVIter::key_type;
    using value_type = V;
    using entry_type = std::pair<key_type, value_type>;

    template <typename Func>
    ReduceIter(KVIter iter1, KVIter iter2, value_type zero, Func f)
        : _iter1(std::move(iter1)),
          _iter2(std::move(iter2)),
          _after_end(false),
          _z(std::move(zero)),
          _f(std::move(f)) { ++(*this); }

    ReduceIter() : _z(value_type()) {}

    ReduceIter(ReduceIter &&) = default;
    ReduceIter& operator=(ReduceIter &&) = default;

    const key_type& key() const {
        return _key;
    }

    const value_type& value() const {
        return _val;
    }

    size_t count() const {
        return _count;
    }

    std::pair<key_type, value_type> operator*() const {
        return std::make_pair(key(), value());
    }

    ReduceIter& operator++() {
        if (_iter1 == _iter2) {
            // so that *this equals to the end()
            _iter1 = KVIter();
            _iter2 = KVIter();
            _after_end = true;
            return *this;
        }
        _key = _iter1.key();
        _val = _z;
        while (_iter1 != _iter2) {
            if (_key != _iter1.key()) {
                break;
            }
            _val = _f(_val, _iter1.value());
            ++_iter1;
            ++_count;
        }
        return *this;
    }

    bool operator==(const ReduceIter &rhs) const {
        return _iter1 == rhs._iter1 && _iter2 == rhs._iter2
            && _after_end == rhs._after_end;
    }

    bool operator!=(const ReduceIter &rhs) const {
        return !(*this == rhs);
    }

    static const ReduceIter& end() {
        static ReduceIter ret;
        return ret;
    }

private:
    KVIter _iter1;
    KVIter _iter2;
    bool _after_end = true;

    entry_type _entry;
    key_type &_key = _entry.first;
    value_type &_val = _entry.second;

    const value_type _z;
    size_t _count = 0;

    std::function<value_type(const value_type&,
            const typename KVIter::value_type&)> _f;
};

} // toppv

#endif // PERSONAL_CODE_TOP_PV_REDUCE_ITER_H
