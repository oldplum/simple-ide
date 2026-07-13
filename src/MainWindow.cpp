#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CodeEditor.h"
#include "Highlighter.h"
#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
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
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F"));
    QAction *newAction = fileMenu->addAction(tr("新建(&N)"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction,
           &QAction::triggered,
           this,
           &MainWindow::newFile);
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
