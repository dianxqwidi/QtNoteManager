#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddFolderClicked();
    void onDeleteFolderClicked();
    void onAddNoteClicked();
    void onDeleteNoteClicked();
    void onSaveNoteClicked();

    void onFolderSelectionChanged();
    void onNoteSelectionChanged();

private:
    Ui::MainWindow *ui;

    struct Note {
        QString title;
        QString content;
    };

    struct Folder {
        QString name;
        QList<Note> notes;
    };

    QList<Folder> folders;
    int currentFolderIndex = -1;
    int currentNoteIndex = -1;

    void loadData();
    void saveData();
    void updateFolderList();
    void updateNoteList();
    void updateNoteEditor();

    QNetworkAccessManager *networkManager;
};

#endif // MAINWINDOW_H
