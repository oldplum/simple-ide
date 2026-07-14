#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class   CodeEditor;
class   QCloseEvent;
class   CatWidget;
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
    bool openFileFromPath(const QString &filePath);
    CatWidget *m_catWidget=nullptr;

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
