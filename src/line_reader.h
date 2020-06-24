/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file kv_iter.h
 * @author qb.wu@outlook.com
 * @date 2020/06/19
 * @brief
 **/

#include <iostream>
#include <fstream>

namespace toppv {

enum ReadStatus : char {
    ONE_LINE_IN_BUF = 0,
    NO_DATA_TO_READ,
    TOO_LARGE_LINE,
    READ_ERROR
};

class LineReader {
public:
    explicit LineReader(size_t buf_len) : _buf_len(buf_len) {}

    LineReader(const LineReader&) = delete;
    LineReader& operator=(const LineReader&) = delete;

    bool init();

    ~LineReader() {
        delete [] _buf;
    }

    ReadStatus scan(std::fstream& fst);

    size_t capacity() const {
        return _buf_len;
    }

    size_t length() const {
        const static size_t EMPTY_LINE_LEN = 0;
        return has_line() ? _line_len : EMPTY_LINE_LEN;
    }

    const char* content() const {
        const static char* EMPTY = "";
        return has_line() ? _buf : EMPTY;
    }

    bool has_line() const {
        return _state == TO_READ_NEXT_LINE && _buf != nullptr;
    }

private:
    enum ReaderState : char {
        TO_READ_NEXT_LINE = 0,
        TO_IGNORE_TOO_LARGE_LINE
    };

    void _clear_reset(ReaderState state) {
        _buf[0]='\0';
        _line_len = 0;
        _state = state;
    }

    size_t _left_len() const {
        return _buf_len - _line_len - 1;
    }

    ReadStatus _read_line(std::fstream &fst);
    ReadStatus _skip_line(std::fstream &fst);

    bool _try_strip_n(std::fstream &fst) const;

    void _read_buf_line(std::fstream &fst);

    const size_t _buf_len = 0;
    char *_buf = nullptr;
    size_t _line_len = 0;

    ReaderState _state = TO_READ_NEXT_LINE;
};

} // toppv
