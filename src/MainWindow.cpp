#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CodeEditor.h"
#include <QCloseEvent>
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
    QMenu *editMenu =menuBar()->addMenu(tr("编辑(&E)"));
    QAction *undoAction = editMenu->addAction(tr("撤销(&U)"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, [this]() {
        CodeEditor *editor = currentEditor();
        if (editor != nullptr) {
            editor->undo();
        }
    });
    QAction *redoAction =editMenu->addAction(tr("重做(&R)"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction,&QAction::triggered,this,[this](){
            CodeEditor *editor=currentEditor();
            if(editor!=nullptr){
                editor->redo();
            }
    });
    editMenu->addSeparator();
    QAction *cutAction=editMenu->addAction(tr("剪切(&X)"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction,&QAction::triggered,this,[this](){
        CodeEditor *editor =currentEditor();
        if(editor!=nullptr){
            editor->cut();
        }
    });
    QAction *copyAction=editMenu->addAction(tr("复制(&C)"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction,&QAction::triggered,this,[this](){
        CodeEditor *editor=currentEditor();
        if(editor!=nullptr){
            editor->copy();
        }
    });
    QAction *pasteAction=editMenu->addAction(tr("复制(&V)"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction,&QAction::triggered,this,[this](){
       CodeEditor *editor=currentEditor();
       if(editor!=nullptr){
           editor->paste();
       }
    });
    editMenu->addSeparator();
    QAction *selectAllAction=editMenu->addAction(tr("全选(&A)"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction,&QAction::triggered,this,[this](){
       CodeEditor *editor=currentEditor();
       if(editor!=nullptr)
       {
           editor->selectAll();
       }
    });
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
    QAction *saveAction=fileMenu->addAction(tr("保存(S)"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction,
            &QAction::triggered,
            this,
            &MainWindow::saveFile);
    QAction *saveAsAction=
            fileMenu->addAction(tr("另存为(&A)"));
    saveAsAction->setShortcut(
                QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_S)
    );
    connect(saveAsAction,
            &QAction::triggered,
            this,
            &MainWindow::saveFileAs);
    QAction *closeAction =fileMenu->addAction(tr("关闭标签页(&W)"));
    closeAction->setShortcuts({
        QKeySequence::Close,
        QKeySequence(Qt::CTRL|Qt::Key_W)}
        );
    connect(closeAction,&QAction::triggered,this,[this](){
       int index=ui->tabWidget->currentIndex();
       if(index!=-1){
           closeTab(index);
       }
    });
    fileMenu->addSeparator();
    QAction *exitAction=fileMenu->addAction(tr("退出(&Q)"));
    exitAction->setShortcuts({
        QKeySequence::Quit,
        QKeySequence(Qt::CTRL|Qt::Key_Q)
        });
    connect(exitAction,
            &QAction::triggered,
            this,
            &MainWindow::close);
    newFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

CodeEditor *MainWindow::currentEditor() const
{
    QWidget *currentPage=ui->tabWidget->currentWidget();
    return qobject_cast<CodeEditor *>(currentPage);
}
void MainWindow::newFile()
{
    CodeEditor *editor=new CodeEditor(ui->tabWidget);
    connect(editor->document(),
            &QTextDocument::modificationChanged,
            this,
            [this,editor](bool modified){
                int index=ui->tabWidget->indexOf(editor);
                if(index==-1)
                    return;
                QString baseName=
                        editor->property("baseName").toString();
                ui->tabWidget->setTabText(
                            index,
                            modified ? baseName+"*":baseName
                                       );
    });
    editor->setProperty("filePath",QString());
    editor->document()->setModified(false);
    QString name = tr("未命名%1").arg(untitledCount);
    ++untitledCount;
    int index=ui->tabWidget->addTab(editor,name);
    ui->tabWidget->setCurrentIndex(index);
    editor->setProperty("baseName",name);
}

void MainWindow::saveFile()
{
    CodeEditor *editor = currentEditor();

    if (editor == nullptr) {
        return;
    }
    QString filePath=editor->property("filePath").toString();
    if(filePath.isEmpty()){
        filePath=QFileDialog::getSaveFileName(
                    this,
                    tr("保存文件"),
                    QString(),
                    tr("代码和文本文件 (*.cpp *.h *.c *.py *.json *.txt);;所有文件 (*.*)")
                    );
    }
    if(filePath.isEmpty()){
        return;
    }
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly|
                  QIODevice::Text|
                  QIODevice::Truncate)){
        QMessageBox::warning(
                    this,
                    tr("保存失败"),
                    tr("无法保存文件:\n%1").arg(filePath)
                    );
        return;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream<<editor->toPlainText();
    file.close();
    editor->setProperty("filePath",filePath);
    QString fileName = QFileInfo(filePath).fileName();
    editor->setProperty("baseName", fileName);
    editor->document()->setModified(false);
    int index=ui->tabWidget->indexOf(editor);
    if(index !=-1){
        ui->tabWidget->setTabText(
                    index,
                    QFileInfo(filePath).fileName()
                    );
    }
}
bool MainWindow::maybeSave(int index)
{
    QWidget *page=ui->tabWidget->widget(index);
    CodeEditor *editor=qobject_cast<CodeEditor*>(page);
    if(editor==nullptr){
        return true;
    }
    if(!editor->document()->isModified()){
        return true;
    }
    QMessageBox::StandardButton result =
            QMessageBox::warning(
                this,
                tr("文件未保存"),
                tr("当前文件有未保存的修改：是否保存？"),
                QMessageBox::Save|
                QMessageBox::Discard|
                QMessageBox::Cancel
                );
   if(result==QMessageBox::Cancel){
       return false;
   }
   if(result==QMessageBox::Discard){
       return true;
   }
   ui->tabWidget->setCurrentIndex(index);
   saveFile();
   return !editor->document()->isModified();
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    for(int index=0;index<ui->tabWidget->count();++index)
    {
        if(!maybeSave(index)){
            event->ignore();
            return;
        }
    }
    event->accept();
}
void MainWindow::closeTab(int index)
{
    if(!maybeSave(index)){
        return;
    }
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
    editor->setPlainText(content);
    editor->setProperty("filePath",filePath);
    editor->document()->setModified(false);
    QString fileName=QFileInfo(filePath).fileName();
    editor->setProperty("baseName",fileName);
    int index=ui->tabWidget->addTab(editor,fileName);
    ui->tabWidget->setCurrentIndex(index);
}
void MainWindow::saveFileAs()
{
    CodeEditor *editor = currentEditor();

    if (editor == nullptr) {
        return;
    }
    QString filePath=QFileDialog::getSaveFileName(
                this,
                tr("另存为"),
                editor->property("filePath").toString(),
                tr("代码和文本文件(*.cpp *.h *.c *.py *.json *.txt);;所有文件(*.*)")
                );
    if(filePath.isEmpty()){
        return;
    }
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly|
                  QIODevice::Text|
                  QIODevice::Truncate)){
        QMessageBox::warning(
                    this,
                    tr("另存为失败"),
                    tr("无法保存文件:\n%1").arg(filePath)
        );
        return;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream<<editor->toPlainText();
    file.close();
    editor->setProperty("filePath",filePath);
    QString fileName = QFileInfo(filePath).fileName();
    editor->setProperty("baseName", fileName);
    editor->document()->setModified(false);
    int index=ui->tabWidget->indexOf(editor);
    if(index !=-1){
        ui->tabWidget->setTabText(
                    index,
                    QFileInfo(filePath).fileName()
                    );
    }
}
