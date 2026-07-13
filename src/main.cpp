#include <QApplication>
//#include "CodeEditor.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("CodeEditor Test");
    window.resize(800, 600);
    window.show();

    return app.exec();
}
