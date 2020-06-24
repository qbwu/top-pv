/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file topk_collector.h
 * @author qb.wu@outlook.com
 * @date 2020/06/22
 * @brief
 **/

#ifndef PERSONAL_CODE_TOP_PV_TOPK_COLLECTOR_H
#define PERSONAL_CODE_TOP_PV_TOPK_COLLECTOR_H

#include <iostream>
#include <functional>
#include <map>

namespace toppv {

class TopKCollector {
    using container_type = std::multimap<
        size_t, std::string, std::greater<size_t>>;
public:
    using iterator_type = container_type::const_iterator;

    explicit TopKCollector(size_t k) : _k(k) {
        if (_k == 0) {
            throw std::invalid_argument("the top k must be greater than 0");
        }
    }

    bool insert(std::string elem, size_t count) {
        if (_map.size() < _k) {
            _map.emplace(count, std::move(elem));
            return true;
        }
        assert(_map.size() > 0 && _map.size() == _k);
        auto min = _map.rbegin()->first;
        if (count <= min) {
            return false;
        }

        _map.emplace(count, std::move(elem));
        _map.erase(--_map.end());
        return true;
    }

    bool empty() const { return _map.empty(); }
    size_t size() const { return _map.size(); }

    iterator_type begin() const {
        return _map.begin();
    }

    iterator_type end() const {
        return _map.end();
    }
private:
    size_t _k = 0;
    container_type _map;
};

} // toppv

#endif // PERSONAL_CODE_TOP_PV_TOPK_COLLECTOR_H

