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
    connect(ui->addNoteButton, &QPushButton::clicked, this, &MainWindow::onAddNoteClicked);
    connect(ui->deleteNoteButton, &QPushButton::clicked, this, &MainWindow::onDeleteNoteClicked);
    connect(ui->saveNoteButton, &QPushButton::clicked, this, &MainWindow::onSaveNoteClicked);
    connect(ui->summarizeButton, &QPushButton::clicked, this, &MainWindow::onSummarizeClicked);

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

void MainWindow::updateFolderList()
{
    ui->folderList->clear();

    for (int folderIndex = 0; folderIndex < folders.size(); ++folderIndex) {
        const Folder &folderObject = folders.at(folderIndex);
        ui->folderList->addItem(folderObject.name);
    }

    if (!folders.isEmpty()) {
        int lastAvailableIndex = folders.size() - 1;
        currentFolderIndex = qMin(currentFolderIndex, lastAvailableIndex);
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
    QFile inputFile("notes.json");
    if (!inputFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray jsonByteArray = inputFile.readAll();
    inputFile.close();

    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonByteArray);
    if (!jsonDocument.isObject()) {
        return;
    }

    folders.clear();

    QJsonObject jsonRootObject = jsonDocument.object();
    QStringList folderNameList = jsonRootObject.keys();

    for (int folderIndex = 0; folderIndex < folderNameList.size(); ++folderIndex) {
        QString folderName = folderNameList.at(folderIndex);
        QJsonValue folderValue = jsonRootObject.value(folderName);
        QJsonArray noteJsonArray = folderValue.toArray();

        Folder folderObject;
        folderObject.name = folderName;

        for (int noteIndex = 0; noteIndex < noteJsonArray.size(); ++noteIndex) {
            QJsonObject noteJsonObject = noteJsonArray.at(noteIndex).toObject();
            Note noteObject;
            noteObject.title = noteJsonObject.value("title").toString();
            noteObject.content = noteJsonObject.value("content").toString();
            folderObject.notes.append(noteObject);
        }

        folders.append(folderObject);
    }
}

void MainWindow::saveData()
{
    QFile outputFile("notes.json");
    if (!outputFile.open(QIODevice::WriteOnly)) {
        return;
    }

    QJsonObject jsonRootObject;

    for (int folderIndex = 0; folderIndex < folders.size(); ++folderIndex) {
        const Folder &folderObject = folders.at(folderIndex);
        QJsonArray noteJsonArray;

        for (int noteIndex = 0; noteIndex < folderObject.notes.size(); ++noteIndex) {
            const Note &noteObject = folderObject.notes.at(noteIndex);
            QJsonObject noteJsonObject;
            noteJsonObject.insert("title", noteObject.title);
            noteJsonObject.insert("content", noteObject.content);
            noteJsonArray.append(noteJsonObject);
        }

        jsonRootObject.insert(folderObject.name, noteJsonArray);
    }

    QJsonDocument jsonDocument(jsonRootObject);
    outputFile.write(jsonDocument.toJson(QJsonDocument::Indented));
    outputFile.close();
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

void MainWindow::onAddNoteClicked()
{
    if (currentFolderIndex < 0 || currentFolderIndex >= folders.size()) return;
    bool ok;
    QString title = QInputDialog::getText(this, "Add Note", "Note title:", QLineEdit::Normal, "", &ok);
    if (ok && !title.isEmpty()) {
        folders[currentFolderIndex].notes.append({title, ""});
        currentNoteIndex = folders[currentFolderIndex].notes.size() - 1;
        updateNoteList();
        updateNoteEditor();
    }
}

void MainWindow::onDeleteNoteClicked()
{
    if (currentFolderIndex < 0 || currentNoteIndex < 0) return;
    folders[currentFolderIndex].notes.removeAt(currentNoteIndex);
    currentNoteIndex = qMin(currentNoteIndex, folders[currentFolderIndex].notes.size() - 1);
    updateNoteList();
    updateNoteEditor();
}

void MainWindow::onSaveNoteClicked()
{
    if (currentFolderIndex < 0 || currentNoteIndex < 0) return;
    folders[currentFolderIndex].notes[currentNoteIndex].content = ui->noteEditor->toPlainText();
    updateNoteList();
    saveData();
}

void MainWindow::onSummarizeClicked()
{
    if (currentFolderIndex < 0 || currentNoteIndex < 0) return;
    QString content = folders[currentFolderIndex].notes[currentNoteIndex].content;
    if (content.isEmpty()) return;
    sendSummaryRequest(content);
}

void MainWindow::sendSummaryRequest(const QString &text)
{
    QString prompt = "Please summarize the following text in English:\n" + text;
    qDebug() << "Prompt sent to Ollama:" << prompt;

    QUrl url("http://localhost:11434/api/generate");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject requestData;
    requestData["model"] = "llama3.2";
    requestData["prompt"] = prompt;
    requestData["stream"] = false;

    QJsonDocument doc(requestData);
    QByteArray jsonData = doc.toJson();

    QNetworkReply* reply = networkManager->post(request, jsonData);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Network error:" << reply->errorString();
            QMessageBox::warning(this, "Error", "Failed to get summary from Ollama.");
            reply->deleteLater();
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);

        if (responseDoc.isObject() && responseDoc.object().contains("response")) {
            QString summary = responseDoc.object().value("response").toString();
            ui->noteEditor->setPlainText(summary);
        } else {
            qDebug() << "Invalid JSON response:" << responseData;
            QMessageBox::warning(this, "Error", "Unexpected response from Ollama.");
        }

        reply->deleteLater();
    });
}

