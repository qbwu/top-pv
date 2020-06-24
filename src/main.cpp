/***************************************************************************
 *
 * Copyright (c) 2020 qbwu All Rights Reserved
 *
 **************************************************************************/

/**
 * @file main.cpp
 * @author qb.wu@outlook.com
 * @date 2020/06/19
 * @brief
 **/

#include <unistd.h>
#include <cassert>

#include <iostream>
#include <thread>
#include <chrono>
#include <exception>
#include <functional>

#include <boost/filesystem.hpp>

#include "mem_table.h"
#include "line_reader.h"
#include "mem_allocator.h"
#include "file_iter.h"
#include "reduce_iter.h"
#include "merged_kv_iter.h"
#include "topk_collector.h"
#include "kv_iter.h"
#include "utils.h"

namespace {

// TODO use gflags or boost::program_options instead
std::string FLAGS_INPUT_FILE;
std::string FLAGS_OUTPUT_FILE;
std::string FLAGS_TMP_DIR;
size_t FLAGS_TOP_K = 100;
size_t FLAGS_LINE_LIMIT = 1024;
size_t FLAGS_BLOCK_SIZE = 4096;
size_t FLAGS_MEM_LIMIT = 1024 * 1024 * 1024;

}

extern char *optarg;
extern int optind, opterr, optopt;

namespace fs = boost::filesystem;

void parse_cmdline(int argc, char **argv) {
    static const std::string usage =
        std::string(argv[0]) +
          "\n\t-f <filename>"
          "\n\t-o <filename>"
          "\n\t[-d <tmp_dir> (default: ./)]"
          "\n\t[-m <mem_limit_byte> (default: 1g)]"
          "\n\t[-l <line_limit_byte> (default: 1k)]"
          "\n\t[-b <block_size_byte> (default: 4k)]"
          "\n\t[-k <top k> (default 100)]\n";

    const char *optstring = "hf:o:d:k:b:m:l:";
    int o = '?';
    try {
        while ((o = getopt(argc, argv, optstring)) != -1) {
            switch (o) {
            case 'h':
                std::cerr << usage << std::endl;
                exit(0);
            case 'f':
                FLAGS_INPUT_FILE = optarg;
                break;
            case 'o':
                FLAGS_OUTPUT_FILE = optarg;
                break;
            case 'd':
                FLAGS_TMP_DIR = optarg;
                break;
            case 'm':
                FLAGS_MEM_LIMIT = std::stoul(optarg);
                break;
            case 'l':
                FLAGS_LINE_LIMIT = std::stoul(optarg);
                break;
            case 'b':
                FLAGS_BLOCK_SIZE = std::stoul(optarg);
                break;
            case 'k':
                FLAGS_TOP_K = std::stoul(optarg);
                break;
            case '?':
                throw std::invalid_argument("error optopt: " + std::to_string(optopt)
                        + ", opterr: " + std::to_string(opterr));
            }
        }
        if (FLAGS_INPUT_FILE.empty() || FLAGS_OUTPUT_FILE.empty()) {
            throw std::invalid_argument("missing input or output file");
        }
        if (FLAGS_LINE_LIMIT > FLAGS_BLOCK_SIZE) {
            throw std::invalid_argument(
                    "line limit should not be larger than block size");
        }
        if (FLAGS_BLOCK_SIZE > FLAGS_MEM_LIMIT) {
            throw std::invalid_argument(
                    "block size should not be larger than mem limit");
        }
        if (FLAGS_TMP_DIR.empty()) {
            FLAGS_TMP_DIR = fs::current_path().string();
        }
    } catch (const std::exception &ex) {
        std::cerr << "cmdline parse error: " << ex.what() << std::endl;
        std::cerr << usage << std::endl;
        exit(-1);
    }
}

void reduce1(const toppv::MemTable &mem_table, uint32_t file_no, size_t &red_num) {

    auto start = std::chrono::steady_clock::now();

    using key_iter = toppv::MemTable::iterator_type;

    struct KeyGetter {
        using type = std::string;
        type operator()(const toppv::MemTable::Record *rec) const {
            return std::string(rec->data, rec->len);
        }
    };

    using value_getter = toppv::Increment<1>;

    using kv_iter = toppv::KVIter<key_iter, KeyGetter, value_getter>;
    using reduce_iter = toppv::ReduceIter<kv_iter, size_t>;

    reduce_iter begin(kv_iter(mem_table.begin()), kv_iter(mem_table.end()),
            0, std::plus<size_t>());

    fs::create_directories(FLAGS_TMP_DIR);

    std::fstream ofs;
    toppv::open_file(FLAGS_TMP_DIR + "/" + toppv::filename(toppv::KV_FILE, file_no),
            std::ios_base::out, ofs);

    size_t num = 0;
    while (begin != reduce_iter::end()) {
        ofs << begin.key() << toppv::KV_SEP << begin.value() << "\n";
        ++begin;
        ++num;
    }

    assert(num <= mem_table.size());
    assert(begin.count() == mem_table.size());

    std::cerr << "reduce1[" << file_no << "] elasped: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count() << "ms, "
        << "number of lines: " << num << std::endl;

    red_num = num;
}

void reduce2(uint32_t file_no, toppv::TopKCollector &collector,
        size_t total, size_t &red_num) {

    auto start = std::chrono::steady_clock::now();

    struct KeyGetter {
        using type = std::string;

        type operator()(const std::string &line) const {
            return line.substr(0, line.find(toppv::KV_SEP));
        }
    };
    struct ValueGetter {
        using type = size_t;

        type operator()(const std::string &line) const {
            return std::stoul(line.substr(line.find(toppv::KV_SEP)));
        }
    };
    using kv_iter = toppv::KVIter<toppv::FileIter, KeyGetter, ValueGetter>;
    using merge_iter = toppv::MergedKVIter<kv_iter>;
    using reduce_iter = toppv::ReduceIter<merge_iter, size_t>;

    std::vector<std::pair<kv_iter, kv_iter>> args;
    args.reserve(file_no);
    for (auto i = 0U; i < file_no; ++i) {
        std::string filename = FLAGS_TMP_DIR + "/" + toppv::filename(toppv::KV_FILE, i);
        toppv::FileIter file_iter(filename);
        if (file_iter != toppv::FileIter::end()) {
            args.emplace_back(
                kv_iter(std::move(file_iter)), kv_iter(toppv::FileIter()));
        }
    }
    reduce_iter begin(merge_iter(std::move(args)), merge_iter(),
                0, std::plus<size_t>());

    size_t num = 0;
    while (begin != reduce_iter::end()) {
        collector.insert(begin.key(), begin.value());
        ++begin;
        ++num;
    }

    assert(begin.count() == total);

    std::cerr << "reduce2 elasped: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count() << "ms, "
        << "number of files: " << file_no << ", "
        << "number of lines: " << num << std::endl;

    red_num = num;
}

int main(int argc, char** argv) {
    parse_cmdline(argc, argv);

    std::fstream ifs;
    toppv::open_file(FLAGS_INPUT_FILE, std::ios_base::in, ifs);

    toppv::LineReader reader(FLAGS_LINE_LIMIT);
    assert(reader.init());

    toppv::MemTable mem_table(FLAGS_BLOCK_SIZE);

    uint32_t file_no = 0;
    size_t count_scan = 0;
    size_t count_red1 = 0;
    size_t count_red2 = 0;
    while (true) {
        auto status = reader.scan(ifs);
        if (status == toppv::ONE_LINE_IN_BUF) {
            ++count_scan;
            mem_table.insert(reader.content(), reader.length());
            if (mem_table.mem_usage() > FLAGS_MEM_LIMIT) {
                // std::this_thread::sleep_for(std::chrono::seconds(2));
                mem_table.sort();
                assert(count_scan == mem_table.size());

                size_t num = 0;
                reduce1(mem_table, file_no++, num);
                count_red1 += num;

                mem_table.clear();
                count_scan = 0;
            }

        } else if (status == toppv::TOO_LARGE_LINE) {
            std::cerr << "skipped too large line" << std::endl;

        } else if (status == toppv::READ_ERROR) {
            std::cerr << "error occured" << std::endl;
            break;

        } else {
            std::cerr << "finished scan" << std::endl;
            break;
        }
    }

    if (!mem_table.empty()) {
        mem_table.sort();
        assert(count_scan == mem_table.size());

        size_t num = 0;
        reduce1(mem_table, file_no++, num);
        count_red1 += num;

        mem_table.clear();
        count_scan = 0;
    }

    toppv::TopKCollector collector(FLAGS_TOP_K);

    reduce2(file_no, collector, count_red1, count_red2);

    assert(collector.size() <= count_red2);

    std::fstream ofs;
    toppv::open_file(FLAGS_OUTPUT_FILE, std::ios_base::out, ofs);
    for (auto iter = collector.begin(); iter != collector.end(); ++iter) {
        ofs << iter->second << "," << iter->first << "\n";
    }
    return 0;
}
