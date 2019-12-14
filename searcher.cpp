#include "searcher.h"

#include <QFile>
#include <QString>
#include <QTextStream>
#include <iostream>
#include "directorywalker.h"
#include <fstream>
#include <deque>
#include <vector>


std::ofstream out("grep-out-real");

bool is_prefix(const std::string& s, size_t suffix_beg) {
        size_t post_i = suffix_beg;
        size_t pref_i = 0;
        for ( ; post_i < s.size(); ++post_i, ++pref_i) {
            if (s[post_i] != s[pref_i])
                return false;
        }
        return true;
}

void preprocess(const std::string& s, std::vector<size_t>& good_suffix_offsets, std::array<size_t, 256>& bad_char_offset) {
    good_suffix_offsets.resize(s.size(), s.size());
    // prefix
    size_t last_pref_len = s.size();
    for (size_t i = s.size(); i > 0; --i) {
        if (is_prefix(s, i)) {
            last_pref_len = s.size() - i;
        }
        // good_suffix_offsets[s.size() - i] = s.size() - last_pref_len;
    }

    // postfix
    for (size_t i = 0; i < s.size() - 1; ++i) {
        size_t post_i = s.size();
        size_t substr_i = i + 1;
        size_t len = 0;
        while (substr_i > 0 && s[post_i - 1] == s[substr_i - 1]) {
            len++;
            post_i--;
            substr_i--;
            good_suffix_offsets[len] = s.size() - i - 1; // len - i + 1;
        }
        good_suffix_offsets[len] = s.size() - i - 1; // len - i + 1;
    }

    for (auto &v : bad_char_offset) {
        v = s.size();
    }
    for (size_t i = 0; i < s.size() - 1; ++i) {
        bad_char_offset[s[i]] = s.size() - i - 1;
    }
}

void test_preprocessing_good_suffix(const std::string& s, const QString& file) {
    std::array<size_t, 256> bad_char;
    std::vector<size_t> good_suffix;
    preprocess(s, good_suffix, bad_char);

    for (size_t i = 0; i < s.size(); ++i) {
        for (size_t k = 0; k < s.size() - i; ++k) {
            std::putchar('?');
        }
        for (size_t k = s.size() - i; k < s.size(); ++k) {
            std::putchar(s[k]);
        }
        std::cout << std::endl;
        std::cout << s << std::endl;
        for (size_t k = 0; k < good_suffix[i]; ++k) {
            std::putchar(' ');
        }
        std::cout << s << std::endl;
    }
    return;
}


void test_preprocessing_bad_char(const std::string& s, const QString& file) {
    std::array<size_t, 256> bad_char;
    std::vector<size_t> good_suffix;
    preprocess(s, good_suffix, bad_char);

    for (size_t i = 0; i < s.size(); ++i) {
        for (size_t j = 'a'; j < 'h'; ++j) {
            for (size_t k = 0; k < s.size() - i - 1; ++k) {
                std::putchar('?');
            }
            std::putchar(j);
            for (size_t k = s.size() - i; k < s.size(); ++k) {
                std::putchar(s[k]);
            }
            std::cout << std::endl;
            std::cout << s << std::endl;
            for (size_t k = 0; k < bad_char[j] - i && bad_char[j] - i <= bad_char[j]; ++k) {
                std::putchar(' ');
            }
            std::cout << s << std::endl;
        }
    }
    return;
}

