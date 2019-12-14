#ifndef SEARCH_H
#define SEARCH_H

#include <QString>
#include <QThreadPool>
#include "taskqueue.h"
#include "directorywalker.h"
#include "searcher.h"
#include <algorithm>

struct Search : public QObject {
    Q_OBJECT
public:
    Search() : fs_finished_(false), dw_finished_(false),
                finish_signal_sendet(false),
                dw_pool(n_directory_walkers),
                fs_pool(n_file_searcher) {
        dw_pool.setFilePool(&fs_pool);
        fs_pool.setDirectoryPool(&dw_pool);
        QObject::connect(&fs_pool, &FileSearcher::pool_finished,
                         this, &Search::fs_finished, Qt::QueuedConnection);
        QObject::connect(&fs_pool, &FileSearcher::new_result,
                         this, &Search::_new_result, Qt::QueuedConnection);
        QObject::connect(&dw_pool, &DirectoryWalker::pool_finished,
                         this, &Search::dw_finished, Qt::QueuedConnection);
    }

    ~Search() {
        dw_pool.finish();
        fs_pool.finish();
        QThreadPool::globalInstance()->waitForDone();
    }

    void search(QList<QString> start_directory, QList<QString> start_files, QString for_search) {
        QList<QString> unique_dirs, unique_files;
        std::sort(start_directory.begin(), start_directory.end(),
                  [](QString const& a, QString& b) { return a.size() < b.size(); } );
        for (auto& t : start_directory) {
            bool recursive = false;
            for (auto& uniq : unique_dirs) {
                QFileInfo info(uniq);
                if (t.startsWith(info.path())) {
                    recursive = true;
                    break;
                }
            }
            if (!recursive)
                unique_dirs.push_back(t);
        }

        for (auto& t : start_files) {
            bool recursive = false;
            for (auto& uniq : unique_dirs) {
                QFileInfo info(uniq);
                if (t.startsWith(info.path())) {
                    recursive = true;
                    break;
                }
            }
            for (auto& uniq : unique_files) {
                QFileInfo info(uniq);
                if (t == uniq) {
                    recursive = true;
                    break;
                }
            }
            if (!recursive)
                unique_files.push_back(t);
        }

        if ((unique_dirs.empty() && unique_files.empty()) || for_search.size() == 0) {
            fs_finished();
            dw_finished();
            check_finish();
            emit search_finished();
            return;
        }

        fs_finished_ = dw_finished_ = finish_signal_sendet = false;
        dw_pool.reset();
        fs_pool.reset();
        dw_pool.start(unique_dirs);
        fs_pool.start(for_search, unique_files);
    }

    void finish() {
        dw_pool.finish();
        fs_pool.finish();
    }

private slots:
    void _new_result(answer s) {
        emit new_result(s);
    }

    void fs_finished() {
        fs_finished_ = true;
        check_finish();
    }

    void dw_finished() {
        dw_finished_ = true;
        check_finish();
    }

    void check_finish() {
        if (fs_finished_ && dw_finished_ && !finish_signal_sendet) {
            finish_signal_sendet = true;
            emit search_finished();
        }
    }

signals:
    void search_finished();
    void new_result(answer s);

private:
    static const size_t n_directory_walkers    = 8;
    static const size_t n_file_searcher        = 8;

    bool fs_finished_, dw_finished_;
    bool finish_signal_sendet;
    DirectoryWalker dw_pool;
    FileSearcher fs_pool;
};

#endif // SEARCH_H
