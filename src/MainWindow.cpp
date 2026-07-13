#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CodeEditor.h"
#include "Highlighter.h"
#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->clear();
    ui->tabWidget->setTabsClosable(true);
    ui->tabWidget->setMovable(true);
    connect(ui->tabWidget,
            &QTabWidget::tabCloseRequested,
            this,
            &MainWindow::closeTab);
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    QAction *newAction = fileMenu->addAction(tr("新建(&N)"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction,
           &QAction::triggered,
           this,
           &MainWindow::newFile);
    QAction *openAction=fileMenu->addAction(tr("打开(&O)"));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction,
            &QAction::triggered,
            this,
            &MainWindow::openFile);
    newFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newFile()
{
    CodeEditor *editor = new CodeEditor(ui->tabWidget);
    new Highlighter(editor->document());  // 绑定语法高亮
    QString name = tr("未命名%1").arg(untitledCount);
    ++untitledCount;
    int index = ui->tabWidget->addTab(editor, name);
    ui->tabWidget->setCurrentIndex(index);
}

void MainWindow::closeTab(int index)
{
    QWidget *page = ui->tabWidget->widget(index);

    ui->tabWidget->removeTab(index);

    if (page != nullptr) {
        page->deleteLater();
    }
}
void MainWindow::openFile()
{
    QString filePath=QFileDialog::getOpenFileName(
                this,
                tr("打开文件"),
                QString(),
                tr("代码和文本文件(*.cpp *.h *.py *.json *.txt);;所有文件(*.*)")
            );
    if(filePath.isEmpty()){
        return;
    }
    QFile file(filePath);
    if(!file.open((QIODevice::ReadOnly|QIODevice::Text))){
        QMessageBox::warning(
                    this,
                    tr("打开失败,"),
                    tr("无法打开文件:\n%1").arg(filePath)
                    );
        return;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString content=stream.readAll();
    file.close();
    CodeEditor *editor=new CodeEditor(ui->tabWidget);
    new Highlighter(editor->document());  // 给新打开的文件绑定语法高亮
    editor->setPlainText(content);
    editor->setProperty("filePath",filePath);
    editor->document()->setModified(false);
    QString fileName=QFileInfo(filePath).fileName();
    int index=ui->tabWidget->addTab(editor,fileName);
    ui->tabWidget->setCurrentIndex(index);
}
