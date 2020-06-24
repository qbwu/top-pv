/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file file_iter.h
 * @author qb.wu@outlook.com
 * @date 2020/06/21
 * @brief
 **/

#ifndef PERSONAL_CODE_TOP_PV_FILE_ITER_H
#define PERSONAL_CODE_TOP_PV_FILE_ITER_H

#include <cerrno>

#include <fstream>

namespace toppv {

class FileIter {
public:
    explicit FileIter(const std::string &fname) : _fst(fname) {
        if (!_fst.good()) {
            throw std::runtime_error("failed to open file: " + fname
                + ", error: " + std::strerror(errno));
        }
        ++(*this);
    }

    FileIter() {
        _fst.close();
    }

    ~FileIter() {
        _fst.close();
    }

    FileIter(FileIter&& rhs) = default;
    FileIter& operator=(FileIter&&) = default;

    const std::string& operator*() const {
        return _line;
    }

    FileIter& operator++() {
        std::getline(_fst, _line);
        if (_fst.eof()) {
            _fst.close();
        }
        return *this;
    }

    // only support to compare with the end iterator
    bool operator==(const FileIter &rhs) const {
        return !_fst.is_open() && !rhs._fst.is_open();
    }

    bool operator!=(const FileIter &rhs) const {
        return !(*this == rhs);
    }

    static const FileIter& end() {
        static FileIter ret;
        return ret;
    }
private:
    std::ifstream _fst;
    std::string _line;
};

} // toppv

#endif // PERSONAL_CODE_TOP_PV_FILE_ITER_H

