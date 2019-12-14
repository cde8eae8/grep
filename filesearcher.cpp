#include "filesearcher.h"

#include "searcher.h"


answer FileSearcher::find(const std::string& s, const QString& file) {
    Buffer<2048> buffer(file.toStdString(), s.size());
    if (!buffer.is_open()) return answer(file, answer::READ_ERROR);
    std::array<size_t, 256> bad_char;
    std::vector<size_t> good_suffix;
    QList<search_result> results;
    preprocess(s, good_suffix, bad_char);

    size_t file_pos = s.size() - 1;
    buffer.next(s.size() - 1);

    while(file_pos < buffer.size() && !finished()) {
        size_t i = 0;
        for ( ; i < s.size() &&
             buffer.get(file_pos - i) == s[s.size() - i - 1]; ++i) { }
        if (i == s.size()) {
            // results.push_back({search_result::RESULT, file_pos - s.size() + 1, 0, ""});
            buffer.found(file_pos - s.size() + 1, file_pos);
            // std::cout << "end = " << file_pos << std::endl;
            file_pos++;
            buffer.next(1);
        } else {
            size_t shift = std::max(static_cast<int64_t>(good_suffix[i]),
                static_cast<int64_t>(bad_char[static_cast<unsigned char>(buffer.get(file_pos - i))] - i));
            file_pos += shift;
            buffer.next(shift);
        }
        if (buffer.binary())
          return answer(file, answer::BINARY);
    }
    return answer(file, buffer.get_results());
}

