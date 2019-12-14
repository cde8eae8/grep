#ifndef FILESEARCHER_H
#define FILESEARCHER_H

#include <QRunnable>
#include <QString>
#include <QThreadPool>
#include <vector>
#include "pool.h"
#include "searcher.h"

#include <iostream>

struct DirectoryWalker;


struct answer {
    enum _state {
        RESULT,
        READ_ERROR,
        BINARY
    } state;

    answer() noexcept {}
    answer(QString filename_, _state st = _state::RESULT) noexcept : state(st), filename(filename_){}
    answer(QString filename_, QList<search_result> res) noexcept : state(_state::RESULT), results(res), filename(filename_) {}

    QList<search_result> results;
    QString filename;
};
Q_DECLARE_METATYPE(answer)

struct FileSearcher : public Pool<QString> {
    Q_OBJECT
public:
    using Pool<QString>::Pool;

    void start(QString s, QList<QString> start_files) {
        this->add_tasks(start_files.begin(), start_files.end());
        for_search = s.toStdString();
        input_finished = false;
        Pool<QString>::start();
    }

    void on_start() override {
        assert(dp);
    }

    void on_finish() override { }

    bool ready_to_finish() override { return input_finished; }

    void process(QString& file) override {
        auto results = find(for_search, file);
        if (results.state == results.RESULT && results.results.empty())
            return;
        emit new_result(results);
    }

    void setInputFinished() {
        input_finished = true;
        if (should_finish())
            finish();
    }
    void setDirectoryPool(DirectoryWalker* dw) { dp = dw; }

    answer find(const std::string& s, const QString& file);


signals:
    void new_result(answer result);

private:
    std::string for_search;
    std::atomic<bool> input_finished;
    DirectoryWalker *dp;
};

#endif // FILESEARCHER_H
