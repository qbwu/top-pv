/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file merged_kv_iter.h
 * @author qb.wu@outlook.com
 * @date 2020/06/22
 * @brief
 **/

#ifndef PERSONAL_CODE_TOP_PV_MERGED_KV_ITER_H
#define PERSONAL_CODE_TOP_PV_MERGED_KV_ITER_H

#include <iostream>
#include <queue>

namespace toppv {

template <typename KVIter>
class MergedKVIter {
    using element_type = std::pair<KVIter, KVIter>;
    using container_type = std::vector<element_type>;

public:
    using iterator_type = typename KVIter::iterator_type;
    using key_type = typename KVIter::key_type;
    using value_type = typename KVIter::value_type;
    using entry_type = std::pair<key_type, value_type>;

    explicit MergedKVIter(container_type kv_iters)
            : _queue(Comparator(), std::move(kv_iters)) {}

    MergedKVIter() = default;

    MergedKVIter(MergedKVIter &&) = default;
    MergedKVIter& operator=(MergedKVIter &&) = default;

    // only support to compare with the end iterator
    bool operator==(const MergedKVIter &rhs) const {
        return _queue.empty() && rhs._queue.empty();
    }

    bool operator!=(const MergedKVIter &rhs) const {
        return !(*this == rhs);
    }

    key_type key() const {
        return _queue.top().first.key();
    }

    value_type value() const {
        return _queue.top().first.value();
    }

    entry_type operator*() const {
        return std::make_pair(key(), value());
    }

    MergedKVIter& operator++() {
        auto min = std::move(const_cast<element_type&>(_queue.top()));
        _queue.pop();
        ++min.first;
        if (min.first != min.second) {
            _queue.push(std::move(min));
        }
        return *this;
    }

    static const MergedKVIter& end() {
        static MergedKVIter ret;
        return ret;
    }

private:
    struct Comparator {
        bool operator()(const element_type &lhs,
                        const element_type &rhs) const {
            return lhs.first.key() >= rhs.first.key();
        }
    };

    std::priority_queue<element_type,
                        container_type,
                        Comparator> _queue;

    entry_type _entry;
};

} // toppv

#endif // PERSONAL_CODE_TOP_PV_MERGED_KV_ITER_H
