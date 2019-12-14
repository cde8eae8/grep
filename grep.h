#ifndef GREP_H
#define GREP_H

#include "search.h"

#include <QMainWindow>
#include <QPainter>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QRect>

QT_BEGIN_NAMESPACE
namespace Ui { class Grep; }
QT_END_NAMESPACE

struct FoundDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        painter->save();
        auto r = option.rect;
        if (index.data().canConvert<search_result>()) {
            QRect bounds;
            search_result result = qvariant_cast<search_result>(index.data());
            size_t pos = 0;
            QRect padding(r);

            auto b = painter->brush();
            auto p = painter->pen();
            painter->setBrush(QColor(60, 60, 60));
            painter->setPen(Qt::NoPen);
            painter->drawRect(r);
            painter->setBrush(b);
            painter->setPen(p);

            for (auto &it : result.offsets) {
                painter->drawText(r, Qt::TextSingleLine,
                                  QString::fromUtf8(result.s.substr(pos, it.line - pos).c_str()), &bounds);  // output char
                r.setX(bounds.x() + bounds.width());
                painter->drawText(r, Qt::TextSingleLine,
                                  QString::fromUtf8(result.s.substr(it.line, it.foundStringLength).c_str()), &bounds);  // output char
                r.setX(bounds.x() + bounds.width());
                painter->drawRect(bounds);
                pos = it.line + it.foundStringLength;
            }
            painter->drawText(r, Qt::TextSingleLine,
                              QString::fromUtf8(result.s.substr(pos, result.s.size() - pos).c_str()), &bounds);  // output char
        } else {
            answer result = qvariant_cast<answer>(index.data());
            if (result.state == result.BINARY) {
                painter->drawText(r, Qt::TextSingleLine, "Binary file " + result.filename + " matches");  // output char
            } else if (result.state == result.READ_ERROR) {
                painter->drawText(r, Qt::TextSingleLine, "Can't open " + result.filename);  // output char
            } else if (result.state == result.RESULT) {
                painter->drawText(r, Qt::TextSingleLine, "Results for " + result.filename);  // output char
            }
        }
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override {

        QFontMetrics fm(option.font);

        if (index.data().canConvert<search_result>()) {
            search_result result = qvariant_cast<search_result>(index.data());
            int pixelsWide = 60 + fm.horizontalAdvance(QString::fromUtf8(result.s.c_str()));
            int pixelsHigh = fm.height() + 5;
            return { pixelsWide, pixelsHigh };
        } else {
            answer result = qvariant_cast<answer>(index.data());
            int pixelsHigh = fm.height();
            int pixelsWide = 0;
            if (result.state == result.BINARY) {
                pixelsWide = fm.horizontalAdvance("Binary file " + result.filename + " matches");
            } else if (result.state == result.READ_ERROR) {
                pixelsWide = fm.horizontalAdvance("Can't open " + result.filename);
            } else if (result.state == result.RESULT) {
                pixelsWide = fm.horizontalAdvance("Results for" + result.filename);
            }
            return { pixelsWide, pixelsHigh };
        }
    }
};

class Grep : public QMainWindow
{
    Q_OBJECT

public:
    Grep(QWidget *parent = nullptr);
    ~Grep();


public slots:
    void searchFinished() {
        searching = false;
    }

    void closing() {
        if (searching)
            s.finish();
        searching = false;
    }

    void new_result(answer result);

private slots:
    void on_start_clicked();

    void search_finished();

    void on_directoryOpenButton_clicked();

    void on_fileOpenButton_clicked();

    void on_removeButton_clicked();

private:
    QStringList dirs, files;
    QStringListModel model;
    Search s;
    bool searching;

    Ui::Grep *ui;
};
#endif // GREP_H
