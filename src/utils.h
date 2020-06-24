/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file utils.h
 * @author qb.wu@outlook.com
 * @date 2020/06/22
 * @brief
 **/

#ifndef PERSONAL_CODE_TOP_PV_UTILS_H
#define PERSONAL_CODE_TOP_PV_UTILS_H

#include <iostream>
#include <fstream>

namespace toppv {

const std::string KV_FILE = "kv";

const std::string KV_SEP = "\t";

inline std::string filename(const std::string &prefix, uint32_t num) {
    char suffix[7];
    snprintf(suffix, sizeof(suffix), "%06d", num);
    return prefix + "." + suffix;
}

inline void open_file(const std::string &fname,
        std::ios_base::openmode mode, std::fstream &fst) {
    fst.open(fname, mode);
    if (!fst.good()) {
        throw std::runtime_error("failed to open file: " + fname);
    }
}

} // toppv

#endif // PERSONAL_CODE_TOP_PV_UTILS_H
