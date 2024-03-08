#include "mainwindow.h"
#include "qtextedit.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextDocument>
#include <QFileDialog>
#include <QTabBar>
#include <QMessageBox>
#include <QTabWidget>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QMainWindow::setWindowTitle("Text Editor");

    tabInit();
    WireConnections();

}

void MainWindow::WireConnections()
{
    connect(ui->actionNew_Tab, &QAction::triggered,this, &MainWindow::OpenNewTab);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::Open);
    connect(ui->actionSave, &QAction::triggered,this, &MainWindow::Save);
    connect(ui->actionSave_As,&QAction::triggered,this,&MainWindow::SaveAs);
    connect(ui->tabWidgetFile,&QTabWidget::tabCloseRequested,this,&MainWindow::closeTab);
    // connect(currentTextEdit,&QTextCursor::, this)


}

void MainWindow::tabInit()
{
    ui->tabWidgetFile->setTabText(0,"New");
    ui->tabWidgetFile->removeTab(1);
    ui->tabWidgetFile->tabsClosable();
    ui->tabWidgetFile->setTabsClosable(true);
}

void MainWindow::closeTab()
{
    int currentTabIndex = ui->tabWidgetFile->currentIndex();

    if(ui->tabWidgetFile->tabText(currentTabIndex).endsWith("*"))
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Save confirmation", "The file '" + ui->tabWidgetFile->tabText(currentTabIndex)
                                                                     + "' has been modified. Do you want to save before closing?", QMessageBox::Save | QMessageBox::Discard);
        if (reply == QMessageBox::Save)
        {
            MainWindow::Save();
        }
    }else{
        connect(ui->tabWidgetFile, &QTabWidget::tabCloseRequested, this, [&](int indexCloseRequested){

            if (indexCloseRequested >= 0 && indexCloseRequested < ui->tabWidgetFile->count())
            {
                ui->tabWidgetFile->removeTab(indexCloseRequested);
            }
        });
    }
}

void MainWindow::Open() {

    int currentTabIndex = ui->tabWidgetFile->currentIndex();
    QTextEdit *currentTextEdit = qobject_cast<QTextEdit*>(ui->tabWidgetFile->currentWidget());

    if (currentTabIndex < 0 )
    {
        qWarning() << "Error: No active tab or text edit found!";
        return;
    }


    QString filePath = QFileDialog::getOpenFileName(
        this, tr("Open File"));

    if (filePath.isEmpty())
    {
        qWarning() << "Error no path found:" << filePath.isEmpty();
        return;
        return;
    }


    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Error opening file:" << file.errorString();
        return;
    }


    QString fileContent = file.readAll();
    if(!currentTextEdit)
    {
        ui->textEditFile->setPlainText(fileContent);
    }else
    {
        currentTextEdit->setPlainText(fileContent);
    }

    QFileInfo fileInfo(file);
    ui->tabWidgetFile->setTabToolTip(currentTabIndex,filePath);
    ui->tabWidgetFile->setTabText(currentTabIndex,fileInfo.fileName());
    connect(currentTextEdit, &QTextEdit::textChanged, this, &MainWindow::OnTextChanged);



    //2 modifs
    connect(ui->textEditFile, &QTextEdit::textChanged, this, [this]() {
        QString tabText0 = this->ui->tabWidgetFile->tabText(0);


        static QString initialText = this->ui->textEditFile->toPlainText();

        if (ui->textEditFile->toPlainText() != initialText)
        {
            if (!tabText0.endsWith("*"))
            {

                this->ui->tabWidgetFile->setTabText(0, tabText0 + "*");
            }

            // Update the initial text for future comparisons
            initialText = ui->textEditFile->toPlainText();

        } else if (tabText0.endsWith("*"))
        {
            this->ui->tabWidgetFile->setTabText(0, tabText0.left(tabText0.length() - 1));
        }
    });
    // --------------------------------------
    file.close();
}

void MainWindow::OpenNewTab()
{
    int tabCount = ui->tabWidgetFile->count();
    QTextEdit& newTab = *new QTextEdit(this);
    QString tabName = "New " + QString::number(tabCount + 1);

    ui->tabWidgetFile->addTab(&newTab,tabName);
    ui->tabWidgetFile->setCurrentWidget(&newTab);
}

void MainWindow::Save()
{
    int index = ui->tabWidgetFile->currentIndex();
    QString tabText = ui->tabWidgetFile->tabText(index);

    if (tabText.endsWith("*")) {
        tabText.chop(1);
        ui->tabWidgetFile->setTabText(index, tabText);
    }

    QString filename = ui->tabWidgetFile->tabToolTip(index);
    qDebug() << "Filename : " <<filename << Qt::endl;

    if (filename.isEmpty()) {
        QMessageBox::warning(this, "Error", "No filename available for saving.");
        return;
    }

    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, "Error Saving File", "Cannot save file: " + file.errorString());
        return;
    }

    QTextStream stream(&file);
    QTextEdit *currentText = qobject_cast<QTextEdit*>(ui->tabWidgetFile->widget(index));

    if (currentText) {
        initialContentMap[currentText] = currentText->toPlainText();
        stream << currentText->toPlainText();
    } else {
        stream << ui->textEditFile->toPlainText();
    }

    file.close();
}



void MainWindow::SaveAs()
{
    int index = ui->tabWidgetFile->currentIndex();
    QString tabText = ui->tabWidgetFile->tabText(index);

    if (tabText.endsWith("*")) {
        tabText.chop(1);
        ui->tabWidgetFile->setTabText(index, tabText);
    }

    QString filename = QFileDialog::getSaveFileName(this, "Save as");
    qDebug() << "Filename : " <<filename << Qt::endl;

    if (filename.isEmpty()) {
        QMessageBox::warning(this, "Error", "No filename available for saving.");
        return;
    }

    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, "Error Saving File", "Cannot save file: " + file.errorString());
        return;
    }

    currentFile = filename;
    setWindowTitle(filename);
    QTextStream stream(&file);
    QTextEdit *currentText = qobject_cast<QTextEdit*>(ui->tabWidgetFile->widget(index));

    if (currentText) {
        initialContentMap[currentText] = currentText->toPlainText();
        stream << currentText->toPlainText();
    } else {
        stream << ui->textEditFile->toPlainText();
    }

    file.close();
}

void MainWindow::OnTextChanged()
{
    int currentTabIndex = ui->tabWidgetFile->currentIndex();
    QString tabText = ui->tabWidgetFile->tabText(currentTabIndex);

    QTextEdit* currentTextEdit = qobject_cast<QTextEdit*>(ui->tabWidgetFile->currentWidget());


    if(currentTextEdit)
    {

        QString initialText = initialContentMap.value(currentTextEdit);

        if(currentTextEdit->toPlainText() == initialText)
        {
            if(tabText.endsWith("*"))
            {
                ui->tabWidgetFile->setTabText(currentTabIndex, tabText.left(tabText.length() - 1));
            }
        }else{
            if(!tabText.endsWith("*"))
            {
                ui->tabWidgetFile->setTabText(currentTabIndex,tabText + "*");
            }
        }
    }


}









MainWindow::~MainWindow()
{
    delete ui;
}







