#include "grep.h"
#include "./ui_grep.h"
#include "search.h"

#include <QFileDialog>
#include <QStringListModel>
#include <QListView>
#include <QStyledItemDelegate>
#include <QPainter>

#include <iostream>

Grep::Grep(QWidget *parent)
    : QMainWindow(parent),
      searching(false),
      ui(new Ui::Grep) {
    qRegisterMetaType<answer>("answer");
    ui->setupUi(this);
    QObject::connect(&this->s, &Search::search_finished,
                     this, &Grep::search_finished);
    QObject::connect(&this->s, &Search::new_result,
                     this, &Grep::new_result);
    ui->outputList->setItemDelegate(new FoundDelegate);
}

Grep::~Grep() {
    delete ui;
}

void Grep::new_result(answer result) {
    if (result.state == result.RESULT) {
        QListWidgetItem *it = new QListWidgetItem;
        it->setData(0, QVariant::fromValue(answer(result.filename, result.state)));
        ui->outputList->addItem(it);
        for (auto &i : result.results) {
            QListWidgetItem *it = new QListWidgetItem;
            it->setData(0, QVariant::fromValue(i));
            ui->outputList->addItem(it);
        }
    } else {
        QListWidgetItem *it = new QListWidgetItem;
        it->setData(0, QVariant::fromValue(result));
        ui->outputList->addItem(it);
    }
}

void Grep::on_start_clicked() {
    if (!searching) {
        searching = true;
        ui->outputList->clear();
        QList<QString> dir = dirs;
        QString for_search = ui->input->text();
        ui->start->setText("Cancel");
        s.search(dir, files, for_search);
    } else {
        s.finish();
    }
}

void Grep::search_finished() {
    ui->start->setText("Search");
    searching = false;
}

void Grep::on_directoryOpenButton_clicked() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec()) {
        dirs.append(dialog.selectedFiles());
        ui->directoriesList->addItems(dialog.selectedFiles());
    }
}

void Grep::on_fileOpenButton_clicked() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    //dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec()) {
        files.append(dialog.selectedFiles());
//        model.setStringList(files);
        ui->directoriesList->addItems(dialog.selectedFiles());
//        ui->directoriesList->setModel(&model);
    }
}

void Grep::on_removeButton_clicked() {
    auto s = ui->directoriesList->selectedItems();
    for (auto &elem : s) {
        QString path = elem->text();
        dirs.removeOne(path);
        files.removeOne(path);
        ui->directoriesList->removeItemWidget(elem);
        delete ui->directoriesList->takeItem(ui->directoriesList->row(elem));
    }
}
