#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CodeEditor.h"
#include "Highlighter.h"
#include "FindReplaceDialog.h"
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
#include <QStatusBar>
#include <QTextCursor>
#include <QDockWidget>
#include <QShortcut>
#include <QApplication>
#include <QSettings>
#include <QStringList>

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
    connect(ui->tabWidget,
            &QTabWidget::currentChanged,
            this,
            [this](int) {
                updateCursorPosition();
            });
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    QMenu *editMenu =menuBar()->addMenu(tr("编辑(&E)"));
    QMenu *viewMenu=menuBar()->addMenu(tr("视图(&V)"));
    QAction *lightThemeAction=
            viewMenu->addAction(tr("浅色主题"));
    connect(lightThemeAction, &QAction::triggered, this, []() {
        qApp->setStyleSheet(QString());
        QSettings settings("SimpleIDE","SimpleIDE");
        settings.setValue("theme","light");
    });
    QAction *darkThemeAction=
            viewMenu->addAction(tr("深色主题"));
    connect(darkThemeAction, &QAction::triggered, this, []() {
        qApp->setStyleSheet(R"(
            QMainWindow {
                background-color: #202124;
            }

            QMenuBar {
                background-color: #2b2b2b;
                color: #eeeeee;
            }

            QMenuBar::item:selected {
                background-color: #3c3f41;
            }

            QMenu {
                background-color: #2b2b2b;
                color: #eeeeee;
            }

            QMenu::item:selected {
                background-color: #3c78d8;
            }

            QTabWidget::pane {
                border: 1px solid #444444;
            }

            QTabBar::tab {
                background-color: #2b2b2b;
                color: #dddddd;
                padding: 6px 12px;
            }

            QTabBar::tab:selected {
                background-color: #3c3f41;
            }

            QPlainTextEdit {
                background-color: #1e1e1e;
                color: #eeeeee;
                selection-background-color: #264f78;
            }

            QStatusBar {
                background-color: #2b2b2b;
                color: #dddddd;
            }

            QDialog {
                background-color: #2b2b2b;
                color: #eeeeee;
            }

            QLabel,
            QCheckBox {
                color: #eeeeee;
            }

            QLineEdit {
                background-color: #3c3f41;
                color: #eeeeee;
                border: 1px solid #555555;
                padding: 4px;
            }

            QPushButton {
                background-color: #3c3f41;
                color: #eeeeee;
                border: 1px solid #555555;
                padding: 5px 12px;
            }

            QPushButton:hover {
                background-color: #4b4f52;
            }
        )");
        QSettings settings("SimpleIDE","SimpleIDE");
        settings.setValue("theme","dark");
    });
    QSettings settings("SimpleIDE","SimpleIDE");
    QString savedTheme=settings.value("theme","light").toString();
    if(savedTheme=="dark")
    {
        darkThemeAction->trigger();
    }
    else
    {
        lightThemeAction->trigger();
    }
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
    QAction *pasteAction=editMenu->addAction(tr("粘贴(&V)"));
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

    editMenu->addSeparator();
    QAction *findAction = editMenu->addAction(tr("查找与替换(&F)"));
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, [this](){
        CodeEditor *editor = currentEditor();
        if (editor != nullptr) {
            FindReplaceDialog *dialog = new FindReplaceDialog(this);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            
            connect(dialog, &FindReplaceDialog::findNext, this, [this](const QString &text, bool caseSensitive, bool wholeWord, bool useRegex, bool backward){
                CodeEditor *curr = currentEditor();
                if (curr) curr->findNext(text, caseSensitive, wholeWord, useRegex, backward);
            });
            connect(dialog, &FindReplaceDialog::replace, this, [this](const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex){
                CodeEditor *curr = currentEditor();
                if (curr) curr->replace(text, replaceText, caseSensitive, wholeWord, useRegex);
            });
            connect(dialog, &FindReplaceDialog::replaceAll, this, [this](const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex){
                CodeEditor *curr = currentEditor();
                if (curr) curr->replaceAll(text, replaceText, caseSensitive, wholeWord, useRegex);
            });
            
            dialog->show();
            dialog->raise();
            dialog->activateWindow();
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

    // 电子猫咪模块初始化
    m_catWidget = new CatWidget(this);
    
    // 用 QDockWidget 将猫咪挂在主界面右侧
    QDockWidget *dock = new QDockWidget(tr("代码伴侣"), this);
    dock->setObjectName("catDock");
    dock->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetClosable);
    dock->setWidget(m_catWidget);
    dock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    // 固定宽度防止太宽
    dock->setMinimumWidth(150);
    dock->setMaximumWidth(200);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // 添加到菜单栏，允许用户随时打开或关闭猫咪
    viewMenu->addAction(dock->toggleViewAction());

    // 绑定快捷键 Ctrl+Shift+F 进行投喂
    QShortcut *feedShortcut = new QShortcut(QKeySequence("Ctrl+Shift+F"), this);
    connect(feedShortcut, &QShortcut::activated, m_catWidget, &CatWidget::feed);

    QSettings sessionSettings("SimpleIDE","SimpleIDE");
    restoreGeometry(sessionSettings.value("window/geometry").toByteArray());
    restoreState(sessionSettings.value("window/state").toByteArray());
    dock->show();
    dock->raise();
    QStringList openFiles=sessionSettings.value("session/openFiles").toStringList();
    QString activeFile=sessionSettings.value("session/activeFile").toString();
    for(const QString &filePath:openFiles){
        openFileFromPath(filePath);
    }
    for(int index=0;index<ui->tabWidget->count();++index)
    {
        CodeEditor *editor =qobject_cast<CodeEditor *>( ui->tabWidget->widget(index));
        if(editor!=nullptr&&editor->property("filePath").toString()==activeFile){
            ui->tabWidget->setCurrentIndex(index);
            break;
        }
    }
    if(ui->tabWidget->count()==0){
        newFile();
    }
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
    CodeEditor *editor = new CodeEditor(ui->tabWidget);
    connect(editor,
            &QPlainTextEdit::cursorPositionChanged,
            this,
            &MainWindow::updateCursorPosition);
    connect(editor, &CodeEditor::bracketMatched, m_catWidget, &CatWidget::onBracketMatched);
    connect(editor, &CodeEditor::codeDeleted, m_catWidget, &CatWidget::onCodeDeleted);
    new Highlighter(editor->document());
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
    int index = ui->tabWidget->addTab(editor, name);
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
    CodeEditor *activeEditor=currentEditor();
    for(int index=0;index<ui->tabWidget->count();++index)
    {
        if(!maybeSave(index)){
            event->ignore();
            return;
        }
    }
    QStringList openFiles;
    for(int index=0;index<ui->tabWidget->count();++index)
    {
        CodeEditor *editor=qobject_cast<CodeEditor*>(ui->tabWidget->widget(index));
        if(editor==nullptr){
            continue;
        }
        QString filePath=editor->property("filePath").toString();
        if(!filePath.isEmpty())
        {
            openFiles.append(filePath);
        }
    }
    QSettings settings("SimpleIDE","SimpleIDE");
    settings.setValue("window/geometry",saveGeometry());
    settings.setValue("window/state",saveState());
    settings.setValue("session/openFiles",openFiles);
    if(activeEditor!=nullptr){
        settings.setValue("session/activeFile",activeEditor->property("filePath").toString());
    }
    else
    {
        settings.remove("session/activeFile");
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
    //QFile file(filePath);
    if(!openFileFromPath(filePath)){
        QMessageBox::warning(
                    this,
                    tr("打开失败,"),
                    tr("无法打开文件:\n%1").arg(filePath)
                    );
        return;
    }
    /*QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString content=stream.readAll();
    file.close();
    CodeEditor *editor=new CodeEditor(ui->tabWidget);
    connect(editor,
            &QPlainTextEdit::cursorPositionChanged,
            this,
            &MainWindow::updateCursorPosition);
    connect(editor, &CodeEditor::bracketMatched, m_catWidget, &CatWidget::onBracketMatched);
    connect(editor, &CodeEditor::codeDeleted, m_catWidget, &CatWidget::onCodeDeleted);
    QString ext = QFileInfo(filePath).suffix();
    new Highlighter(editor->document(), ext);

    editor->setPlainText(content);
    editor->setProperty("filePath",filePath);
    editor->document()->setModified(false);
    QString fileName=QFileInfo(filePath).fileName();
    editor->setProperty("baseName",fileName);
    int index=ui->tabWidget->addTab(editor,fileName);
    ui->tabWidget->setCurrentIndex(index);*/
}
bool MainWindow::openFileFromPath(const QString &filePath)
{
    if(filePath.isEmpty()||!QFileInfo::exists(filePath)){
        return false;
    }
    for(int index=0;index<ui->tabWidget->count();++index)
    {
        CodeEditor *editor=qobject_cast<CodeEditor*>(ui->tabWidget->widget(index));
        if(editor!=nullptr&&editor->property("filePath").toString()==filePath){
            ui->tabWidget->setCurrentIndex(index);
            return true;
        }
    }
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString content=stream.readAll();
    file.close();
    CodeEditor *editor=new CodeEditor(ui->tabWidget);
    connect(editor,&QPlainTextEdit::cursorPositionChanged,this,&MainWindow::updateCursorPosition);
    connect(editor, &CodeEditor::bracketMatched, m_catWidget, &CatWidget::onBracketMatched);
    connect(editor, &CodeEditor::codeDeleted, m_catWidget, &CatWidget::onCodeDeleted);
    QString extension=QFileInfo(filePath).suffix();
    new Highlighter(editor->document(),extension);
    editor->setPlainText(content);
    editor->setProperty("filePath",filePath);
    QString fileName=QFileInfo(filePath).fileName();
    editor->setProperty("baseName",fileName);
    editor->document()->setModified(false);
    int index=ui->tabWidget->addTab(editor,fileName);
    ui->tabWidget->setCurrentIndex(index);
    return true;
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
void MainWindow::updateCursorPosition()
{
    CodeEditor *editor=currentEditor();
    if(editor==nullptr)
    {
        statusBar()->showMessage(tr("就绪"));
        return;
    }
    QTextCursor cursor = editor->textCursor();
    int line=cursor.blockNumber()+1;
    int column=cursor.positionInBlock()+1;
    statusBar()->showMessage(
                tr("第%1行，第%2列").arg(line).arg(column)
                );
}
