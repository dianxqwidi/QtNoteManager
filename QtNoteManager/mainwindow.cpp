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

    connect(ui->addFolderButton, &QPushButton::clicked, this, &MainWindow::onAddFolderClicked);
    connect(ui->deleteFolderButton, &QPushButton::clicked, this, &MainWindow::onDeleteFolderClicked);

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

void MainWindow::loadData()
{
    QFile file("notes.json");
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    folders.clear();
    QJsonObject root = doc.object();
    for (const QString &key : root.keys()) {
        Folder folder;
        folder.name = key;
        QJsonArray notesArray = root[key].toArray();
        for (const QJsonValue &noteVal : notesArray) {
            QJsonObject noteObj = noteVal.toObject();
            Note note;
            note.title = noteObj["title"].toString();
            note.content = noteObj["content"].toString();
            folder.notes.append(note);
        }
        folders.append(folder);
    }
}

void MainWindow::saveData()
{
    QFile file("notes.json");
    if (!file.open(QIODevice::WriteOnly)) return;

    QJsonObject root;
    for (const Folder &folder : folders) {
        QJsonArray notesArray;
        for (const Note &note : folder.notes) {
            QJsonObject obj;
            obj["title"] = note.title;
            obj["content"] = note.content;
            notesArray.append(obj);
        }
        root[folder.name] = notesArray;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
}

void MainWindow::onAddFolderClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Add Folder", "Folder name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        folders.append({name, {}});
        currentFolderIndex = folders.size() - 1;
        updateFolderList();
        updateNoteList();
    }
}

void MainWindow::onDeleteFolderClicked()
{
    if (currentFolderIndex < 0 || currentFolderIndex >= folders.size()) return;
    folders.removeAt(currentFolderIndex);
    currentFolderIndex = qMin(currentFolderIndex, folders.size() - 1);
    updateFolderList();
    updateNoteList();
    updateNoteEditor();
}
