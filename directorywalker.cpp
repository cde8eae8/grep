#include "directorywalker.h"


void DirectoryWalker::process(QString &dir_path) {
    QDirIterator it(dir_path, QDir::Readable | QDir::Dirs |
                    QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden |
                    QDir::NoSymLinks);

    while (it.hasNext()) {
        QString entry = it.next();
        QFileInfo info = it.fileInfo();
        if (info.isDir()) {
            add_task(entry);
        } else if (info.isFile()) {
            filePool->add_task(entry);
        }
    }
}
