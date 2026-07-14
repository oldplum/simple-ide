#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "CatWidget.h"

class   CodeEditor;
class   QCloseEvent;
namespace Ui{
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    int untitledCount=1;
    CodeEditor *currentEditor() const;
    bool maybeSave(int index);
    CatWidget *m_catWidget;

private slots:
    void newFile();
    void openFile();
    void closeTab(int index);
    void saveFile();
    void saveFileAs();
    void updateCursorPosition();

signals:

};

#endif // MAINWINDOW_H
