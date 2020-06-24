/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file line_reader.cpp
 * @author qb.wu@outlook.com
 * @date 2020/06/19
 * @brief
 **/

#include "line_reader.h"

#include <cassert>

namespace toppv {

bool LineReader::init() {
    if (_buf_len == 0) {
        return false;
    }
    if (_buf == nullptr) {
        _buf = new (std::nothrow) char[_buf_len];
        if (_buf == nullptr) {
            return false;
        }
    }
    _clear_reset(TO_READ_NEXT_LINE);
    return true;
}

ReadStatus LineReader::scan(std::fstream& fst) {
    if (_buf == nullptr) {
        return READ_ERROR;
    }

    auto res = READ_ERROR;
    _clear_reset(_state);
    switch (_state) {
        case TO_READ_NEXT_LINE:
            res = _read_line(fst);
            break;
        case TO_IGNORE_TOO_LARGE_LINE:
            res = _skip_line(fst);
            break;
        default:
            std::cerr << "Unkown state: " << _state << "\n";
            res = READ_ERROR;
    }
    return res;
}

ReadStatus LineReader::_read_line(std::fstream& fst) {
    _read_buf_line(fst);
    if (fst.eof()) {
        _clear_reset(TO_READ_NEXT_LINE);
        return NO_DATA_TO_READ;
    }

    if (_left_len() == 0) {
        _clear_reset(TO_IGNORE_TOO_LARGE_LINE);
        return TOO_LARGE_LINE;
    } else { // _left_len() > 0
        if (_try_strip_n(fst)) {
            _state = TO_READ_NEXT_LINE;
            // don't clear the buffer
            return ONE_LINE_IN_BUF;
        }
        _clear_reset(TO_READ_NEXT_LINE);
        return READ_ERROR;
    }
}

ReadStatus LineReader::_skip_line(std::fstream& fst) {
    _read_buf_line(fst);
    if (fst.eof()) {
        _clear_reset(TO_READ_NEXT_LINE);
        return NO_DATA_TO_READ;
    }

    if (_left_len() == 0) {
        _clear_reset(TO_IGNORE_TOO_LARGE_LINE);
        return TOO_LARGE_LINE;
    } else { // _left_len() > 0
        _clear_reset(TO_READ_NEXT_LINE);
        if (_try_strip_n(fst)) {
            return TOO_LARGE_LINE;
        }
        return READ_ERROR;
    }
}

bool LineReader::_try_strip_n(std::fstream &fst) const {
    if (fst.fail()) {
        // clear possible IO errors, like having read an empty line
        fst.clear();
    }
    auto c = fst.get();
    if (fst.good() && c != '\n') {
        fst.put(c);
    }
    return c == '\n';
}

void LineReader::_read_buf_line(std::fstream &fst) {
    fst.get(_buf, _left_len() + 1, '\n');

    assert(static_cast<size_t>(fst.gcount()) <= _left_len());

    _line_len = fst.gcount();
    _buf[_line_len] = '\0';
}

} // toppv
