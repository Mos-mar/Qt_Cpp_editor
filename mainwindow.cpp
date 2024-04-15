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
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    settings = new QSettings("Organization","Text Editor",this);
    QMainWindow::setWindowTitle("Text Editor");

    for (int i = 0; i < MAXRECENTFILE; ++i)
    {
        QAction *action = new QAction(this);
        action->setVisible(false);        
        ui->menuRecent_Files->addAction(action);
        recentFiles.append(action);
    }
    UpdateRecentFileActions();
    tabInit();
    WireConnections();
    statusBar()->showMessage(tr("Ready"));

}

void MainWindow::WireConnections()
{
    QTextEdit *currentTextEdit = qobject_cast<QTextEdit*>(ui->tabWidgetFile->currentWidget());
    connect(currentTextEdit,&QTextEdit::cursorPositionChanged,this,&MainWindow::cursorPosition);
    connect(ui->textEditFile,&QTextEdit::cursorPositionChanged,this,&MainWindow::cursorPositionDefaultTab);
    connect(ui->actionNew_Tab, &QAction::triggered,this, &MainWindow::OpenNewTab);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::Open);
    connect(ui->actionSave, &QAction::triggered,this, &MainWindow::Save);
    connect(ui->actionSave_As,&QAction::triggered,this,&MainWindow::SaveAs);
    connect(ui->tabWidgetFile,&QTabWidget::tabCloseRequested,this,&MainWindow::closeTab);
    connect(ui->actionSearchReplace,&QAction::triggered,this,&MainWindow::SearchReplace);    
    for (QAction *action : recentFiles)
    {
        connect(action, &QAction::triggered, this, &MainWindow::OpenRecentFile);
    }
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

void MainWindow::cursorPosition()
{
    QTextEdit* currentTextEdit = qobject_cast<QTextEdit*>(ui->tabWidgetFile->currentWidget());
    if(currentTextEdit)
    {
        QTextCursor cursor = currentTextEdit->textCursor();
        int row = cursor.blockNumber() + 1;
        int column = cursor.columnNumber() + 1;
        statusBar()->showMessage(tr("Row %1, Column %2").arg(row).arg(column));
    }else{
        statusBar()->clearMessage();
    }

}

void MainWindow::cursorPositionDefaultTab()
{
    if(ui->textEditFile)
    {
        QTextCursor cursor0 = ui->textEditFile->textCursor();
        int row = cursor0.blockNumber() + 1;
        int column = cursor0.columnNumber() + 1;
        statusBar()->showMessage(tr("Row %1 , Column %2").arg(row).arg(column));
    }else{
        statusBar()->clearMessage();
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
    connect(currentTextEdit,&QTextEdit::cursorPositionChanged,this,&MainWindow::cursorPosition);
    connect(ui->textEditFile,&QTextEdit::cursorPositionChanged,this,&MainWindow::cursorPositionDefaultTab);

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
    //Adds the file to the list of last files opened (in my private attribute "settings")
    QStringList recentFile = settings->value("RecentFiles").toStringList();
    recentFile.removeAll(filePath);
    recentFile.prepend(filePath);
    while (recentFile.size() > 10)
        recentFile.removeLast();
    settings->setValue("RecentFiles", recentFile);
    UpdateRecentFileActions();
    file.close();
}

void MainWindow::OpenNewTab()
{
    QTextEdit *currentTextEdit = qobject_cast<QTextEdit*>(ui->tabWidgetFile->currentWidget());
    connect(currentTextEdit,&QTextEdit::cursorPositionChanged,this,&MainWindow::cursorPosition);
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
    QTextEdit *currentTextEdit = qobject_cast<QTextEdit*>(ui->tabWidgetFile->widget(index));

    if (currentTextEdit) {
        initialContentMap[currentTextEdit] = currentTextEdit->toPlainText();
        stream << currentTextEdit->toPlainText();
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

void MainWindow::SearchReplace()
{
    QTextEdit* currentTextEdit = qobject_cast<QTextEdit*>(ui->tabWidgetFile->currentWidget());

    QDialog dialog(this);
    dialog.setWindowTitle("Search");

    QGridLayout layout;
    dialog.setLayout(&layout);
    dialog.resize(400 , 300);

    QLabel searchLabel("Search for: ");
    QLineEdit searchLine;
    QLabel replaceLabel("Replace : ");
    QLineEdit replaceLine;

    QPushButton searchBtn("Search");
    QPushButton replaceBtn("Replace");
    QPushButton replaceAllBtn("Replace All");

    layout.addWidget(&searchLabel,0,0);
    layout.addWidget(&searchLine,0,1);
    layout.addWidget(&replaceLabel,2,0);
    layout.addWidget(&replaceLine,2,1);
    layout.addWidget(&searchBtn,1,0);
    layout.addWidget(&replaceBtn,3,0);
    layout.addWidget(&replaceAllBtn,3,1);
    replaceAllBtn.setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    if(currentTextEdit)
    {
        QTextDocument::FindFlag flag = QTextDocument::FindCaseSensitively;
        connect(&searchBtn,&QPushButton::clicked,this, [&] {

            if(currentTextEdit->find(searchLine.text(),flag))
            {

                QTextCursor currentCursor = currentTextEdit->textCursor();
                QTextCharFormat highLightFormat;
                highLightFormat.setBackground(Qt::yellow);
                highLightFormat.setForeground(Qt::black);
                currentCursor.mergeCharFormat(highLightFormat);
                currentCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            }

        });

        connect(&replaceBtn,&QPushButton::clicked,this, [&] {
            if(currentTextEdit->textCursor().hasSelection())
            {
                currentTextEdit->insertPlainText(replaceLine.text());
            }

        });

        connect(&replaceAllBtn,&QPushButton::clicked,this, [&] {
            currentTextEdit->moveCursor(QTextCursor::Start);
            QTextCursor currentCursor = currentTextEdit->textCursor();
            QTextCursor searched = currentTextEdit->document()->find(searchLine.text(),flag);
            while(!searched.isNull())
            {
                searched.insertText(replaceLine.text());
                searched = currentTextEdit->document()->find(searchLine.text(),flag);
            }


        });
    }else if(!currentTextEdit)
    {
        QTextDocument::FindFlag flag = QTextDocument::FindCaseSensitively;
        connect(&searchBtn,&QPushButton::clicked,this, [&] {

            if(ui->textEditFile->find(searchLine.text(),flag))
            {

                QTextCursor currentCursor = ui->textEditFile->textCursor();
                QTextCharFormat highLightFormat;
                highLightFormat.setBackground(Qt::yellow);
                highLightFormat.setForeground(Qt::black);
                currentCursor.mergeCharFormat(highLightFormat);
                currentCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            }

        });

        connect(&replaceBtn,&QPushButton::clicked,this, [&] {
            if(ui->textEditFile->textCursor().hasSelection())
            {
                ui->textEditFile->insertPlainText(replaceLine.text());
            }

        });

        connect(&replaceAllBtn,&QPushButton::clicked,this, [&] {
            ui->textEditFile->moveCursor(QTextCursor::Start);
            QTextCursor currentCursor = ui->textEditFile->textCursor();
            QTextCursor searched = ui->textEditFile->document()->find(searchLine.text(),flag);
            while(!searched.isNull())
            {
                searched.insertText(replaceLine.text());
                searched = ui->textEditFile->document()->find(searchLine.text(),flag);
            }


        });
    }

    dialog.exec();
}



void MainWindow::OpenRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());

    if (action)
    {        
        QString fileName = action->data().toString();
        qDebug() << "filename " <<  fileName << Qt::endl;
        displayRecent(fileName);
    }
}

void MainWindow::displayRecent(const QString &fileName)
{
    QAction* action = qobject_cast<QAction*>(sender());


    if(action)
    {
        //New tab with corresponding tabName of chosen recent file
        QTextEdit* newTab = new QTextEdit(this);
        QFileInfo fileInfo(fileName);
        QString tabName =  fileInfo.fileName();
        ui->tabWidgetFile->addTab(newTab,tabName);
        ui->tabWidgetFile->setCurrentWidget(newTab);

        //filling current text edit with the content of the file
        QFile recentFile(fileName);

        if(recentFile.exists() && recentFile.open(QIODevice::ReadOnly))
        {
            int currentTabIndex = ui->tabWidgetFile->indexOf(newTab);
            ui->tabWidgetFile->setTabToolTip(currentTabIndex,fileName); //to enable save fct
            connect(newTab,&QTextEdit::cursorPositionChanged,this,&MainWindow::cursorPosition);
            QString recentFileContent = recentFile.readAll();
            newTab->setPlainText(recentFileContent);
            connect(newTab, &QTextEdit::textChanged, this, &MainWindow::OnTextChanged);

            recentFile.close();
        }else{
            qDebug() << "Could not open file in MainWindow::displayRecent" << recentFile.errorString();
        }
    }
}


void MainWindow::UpdateRecentFileActions()
{
    QStringList files = settings->value("RecentFiles").toStringList();
    int numRecentFiles = qMin(files.size(), MAXRECENTFILE);
    qDebug() << "numrecent files : " << numRecentFiles << Qt::endl;

    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
        recentFiles[i]->setText(text);
        recentFiles[i]->setData(files[i]);
        recentFiles[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < MAXRECENTFILE; ++j) {
        recentFiles[j]->setVisible(false);
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

