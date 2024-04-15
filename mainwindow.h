#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include<QList>
#include<QSettings>
#include<QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:


private slots:   
    void OpenNewTab();
    void Open();
    void Save();
    void SaveAs();
    void closeTab();
    void cursorPosition();
    void cursorPositionDefaultTab();
    void SearchReplace();
    void OpenRecentFile();
    void UpdateRecentFileActions();
    void displayRecent(const QString& fileName);


private:
    Ui::MainWindow *ui;
    QMap<QTextEdit*, QString> initialContentMap;
    QString currentFile = "";
    QSettings* settings;
    static const int MAXRECENTFILE = 10;
    QList<QAction*> recentFiles;
    void WireConnections();
    void tabInit();
    void OnTextChanged();


};
#endif // MAINWINDOW_H
