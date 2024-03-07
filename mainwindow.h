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




private:
    Ui::MainWindow *ui;
    QMap<QTextEdit*, QString> initialContentMap;
    void WireConnections();
    void tabInit();
    void OnTextChanged();
    void OnTextChanged0(); //vire ca
};
#endif // MAINWINDOW_H
