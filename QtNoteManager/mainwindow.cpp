#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);

    connect(ui->folderList, &QListWidget::currentRowChanged, this, &MainWindow::onFolderSelectionChanged);
    connect(ui->noteList, &QListWidget::currentRowChanged, this, &MainWindow::onNoteSelectionChanged);

    loadData();
    updateFolderList();
}

MainWindow::~MainWindow()
{
    saveData();
    delete ui;
}

void MainWindow::updateFolderList() {
    ui->folderList->clear();
    for (const Folder &folder : folders)
        ui->folderList->addItem(folder.name);
    if (!folders.isEmpty()) {
        currentFolderIndex = qMin(currentFolderIndex, folders.size() - 1);
        ui->folderList->setCurrentRow(currentFolderIndex);
    }
}

void MainWindow::updateNoteList() {
    ui->noteList->clear();
    if (currentFolderIndex < 0 || currentFolderIndex >= folders.size()) return;
    const Folder &folder = folders[currentFolderIndex];
    for (const Note &note : folder.notes)
        ui->noteList->addItem(note.title);
    if (!folder.notes.isEmpty()) {
        currentNoteIndex = qMin(currentNoteIndex, folder.notes.size() - 1);
        ui->noteList->setCurrentRow(currentNoteIndex);
    }
}

void MainWindow::updateNoteEditor() {
    if (currentFolderIndex < 0 || currentFolderIndex >= folders.size() ||
        currentNoteIndex < 0 || currentNoteIndex >= folders[currentFolderIndex].notes.size()) {
        ui->noteEditor->clear();
        return;
    }
    ui->noteEditor->setPlainText(folders[currentFolderIndex].notes[currentNoteIndex].content);
}

void MainWindow::onFolderSelectionChanged() {
    currentFolderIndex = ui->folderList->currentRow();
    currentNoteIndex = -1;
    updateNoteList();
    updateNoteEditor();
}

void MainWindow::onNoteSelectionChanged() {
    currentNoteIndex = ui->noteList->currentRow();
    updateNoteEditor();
}
