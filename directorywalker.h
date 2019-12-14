#ifndef DIRECTORYWALKER_H
#define DIRECTORYWALKER_H

#include <QDirIterator>
#include <QRunnable>
#include <QString>
#include <QThreadPool>
#include <vector>
#include "taskqueue.h"
#include "filesearcher.h"
#include "pool.h"

struct DirectoryWalker : public Pool<QString> {
public:
    using Pool<QString>::Pool;

    void setFilePool(FileSearcher* fp) {
        filePool = fp;
    }

    void start(QList<QString> initialValue) {
        add_tasks(initialValue.begin(), initialValue.end());
        Pool<QString>::start();
    }

    void on_start() override {
        assert(filePool);
    }

    void process(QString& dir_path) override;

    virtual void all_finished() override {
        filePool->setInputFinished();
    }


private:
    FileSearcher *filePool;
};


#endif // DIRECTORYWALKER_H
