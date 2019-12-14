#ifndef SEARCHER_H
#define SEARCHER_H

#include <QList>
#include <iostream>
#include <fstream>

#include <QFile>
#include <QString>
#include <QTextStream>
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>

inline bool is_binary(char c) {
    return !((c <= 'z' && c >= ' ') || (c >= '\b' && c <= '\r'));
}

struct search_result {
    struct offset {
      offset(size_t a, size_t b, size_t str_length) noexcept :
          line(a),
          file(b),
          foundStringLength(str_length) {
      }
      size_t line;
      size_t file;
      size_t foundStringLength;
    };

    std::vector<offset> offsets;
    std::string s;
};

Q_DECLARE_METATYPE(search_result)


template <size_t _BUFFER_SIZE>
struct Buffer {
    Buffer(std::string file, size_t saved_size_)
          : REAL_BUFFER_SIZE(std::max(_BUFFER_SIZE, saved_size_)),
            current_buffer(REAL_BUFFER_SIZE),
            current_buffer_file_pos(0),
            prev_buffer(REAL_BUFFER_SIZE),
            saved_size(saved_size_),
            in(file), file_pos(0),
            search_for_newline(false),
            binary_(false)
    {
        // assert(in.is_open());
        in.seekg(0, std::ios_base::end);
        file_size = in.tellg();
        in.seekg(0, std::ios_base::beg);
        read();
    }

    ~Buffer() {
        in.close();
    }

    bool is_open() {
      return in.is_open();
    }

    char next(size_t shift) {
        char c = _next(shift);
        return c;
    }

    char get(size_t pos) {
        assert(pos <= file_pos);
        assert(pos > file_pos - saved_size || (pos >= 0 && file_pos < saved_size));

        if (pos >= current_buffer_file_pos)
            return current_buffer.data()[pos - current_buffer_file_pos];
        else
            return prev_buffer
                    .data()[prev_buffer.size + pos - current_buffer_file_pos];

    }

    int64_t rfind_newline(char* begin, size_t size) {
        for (char* i = begin + size; i != begin; --i) {
            if (is_binary(*(i - 1)))
                binary_ = true;
            if (*(i - 1) == '\n')
                return i - 1 - begin;
        }
        return -1;
    }

    int64_t lfind_newline(char* begin, char* end) {
        char* i = begin;
        for ( ; i != end; ++i) {
            if (is_binary(*i))
                binary_ = true;
            if (*i == '\n')
                break;
        }
        return i - begin;
    }

    void find_prev_newline(size_t pos, size_t len) {
        if (search_for_newline)
            return;
        else {
            results.push_back({});
        }
        auto& s = results.back();
        auto cur_pos = rfind_newline(current_buffer.data(), pos - current_buffer_file_pos);
        s.s.insert(s.s.begin(), &current_buffer.data()[cur_pos + 1], &current_buffer.data()[pos - current_buffer_file_pos]);
        if (cur_pos != -1) {
            s.offsets.emplace_back(pos - cur_pos - current_buffer_file_pos - len, pos - len + 1, len);
            return;
        }
        cur_pos = rfind_newline(prev_buffer.data(), prev_buffer.size);
        s.s.insert(s.s.begin(), &prev_buffer.data()[cur_pos + 1], &prev_buffer.data()[prev_buffer.size]);
        if (cur_pos != -1) {
            s.offsets.emplace_back(pos - (current_buffer_file_pos - prev_buffer.size + cur_pos) - len, pos - len + 1, len);
            return;
        }

        inner_buffer b(REAL_BUFFER_SIZE);
        size_t b_pos = current_buffer_file_pos - prev_buffer.size;
        size_t old_filepos = in.tellg();
        auto state = in.rdstate();
        bool found = false;
        while (b_pos > 0) {
            b_pos -= b.max_size;
            in.seekg(b_pos);
            in.read(b.data(), b.max_size);
            b.size = in.gcount();
            cur_pos = rfind_newline(b.data(), b.size);
            s.s.insert(s.s.begin(), &b.data()[cur_pos + 1], &b.data()[b.size]);
            if (cur_pos != -1) {
                s.offsets.emplace_back(pos - (b_pos + cur_pos) - len, pos - len + 1, len);
                found = true;
                break;
            }
        }
        if (!found) {
            s.offsets.emplace_back(pos - len + 1, pos - len + 1, len);
        }
        in.seekg(old_filepos);
        in.setstate(state);
    }

    void found(size_t beg, size_t end) {
        if (!results.empty() && !results.back().offsets.empty()) {
          auto& lastres = results.back();
          size_t lastres_begin = lastres.offsets.back().file - lastres.offsets.back().line;
          if (beg < lastres_begin + lastres.s.size() || search_for_newline) {
              lastres.offsets.emplace_back(beg - (lastres.offsets.back().file - lastres.offsets.back().line), beg, end - beg + 1);
              return;
          }
        }
        find_prev_newline(end, end - beg + 1);
        if (search_for_newline) return;
        search_for_newline = true;
        end -= current_buffer_file_pos;
        auto pos = lfind_newline(current_buffer.data() + end, current_buffer.data() + current_buffer.size);
        accumulated_line.append(current_buffer.data() + end, pos);
        if (pos != current_buffer.size - end) {
            search_for_newline = false;
            auto& last_found = results.back();
            last_found.s.append(accumulated_line);
            accumulated_line.clear();
        }
    }

    auto get_results() {
        return std::move(results);
    }

    bool eof() {
        return in.eof() && file_pos >= current_buffer.size;
    }

    size_t size() {
        return file_size;
    }

    bool binary() {
      return binary_;
    }


private:
    int _next(size_t shift) {
        size_t new_pos = file_pos + shift;
        while (new_pos >= current_buffer_file_pos + current_buffer.size
               && !in.eof()) {
            read();
        }
        file_pos = new_pos;
        if (eof())
            return EOF;
        assert(file_pos - current_buffer_file_pos >= 0);
        assert(file_pos - current_buffer_file_pos < current_buffer.size);
        return current_buffer.buffer[file_pos - current_buffer_file_pos];
    }

    void read() {
        swap_buffers();
        in.read(current_buffer.data(), current_buffer.max_size);
        current_buffer.size = in.gcount();
        current_buffer_file_pos += prev_buffer.size;
        if (search_for_newline) {
            auto pos = lfind_newline(current_buffer.data(), current_buffer.data() + current_buffer.size);
            accumulated_line.append(current_buffer.data(), pos);
            if (pos != current_buffer.size) {
                search_for_newline = false;
                auto& last_found = results.back();
                last_found.s.append(accumulated_line);
                accumulated_line.clear();
            }
        }
    }

    void swap_buffers() {
        std::swap(prev_buffer.buffer, current_buffer.buffer);
        std::swap(prev_buffer.size, current_buffer.size);
    }

    struct inner_buffer {
        char *buffer;
        size_t size;
        size_t max_size;
        inner_buffer(size_t size_) : max_size(size_), size(0) {
            buffer = new char[max_size];
        }

        char* data() {
            return buffer;
        }

        ~inner_buffer() {
            delete [] buffer;
        }
    };

    const size_t REAL_BUFFER_SIZE;
    inner_buffer prev_buffer, current_buffer;
    size_t file_pos;
    size_t current_buffer_file_pos;

    std::ifstream in;
    size_t file_size;
    const size_t saved_size;

    QList<search_result> results;
    bool search_for_newline;
    std::string accumulated_line;
    bool binary_;
};

void preprocess(const std::string& s, std::vector<size_t>& good_suffix_offsets, std::array<size_t, 256>& bad_char_offset);


void test_preprocessing_bad_char(const std::string& s, const QString& file);
void test_preprocessing_good_suffix(const std::string& s, const QString& file);

#endif // SEARCHER_H
