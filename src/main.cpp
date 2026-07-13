#include <QApplication>
#include "CodeEditor.h"
#include "Highlighter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    CodeEditor editor;
    Highlighter *highlighter = new Highlighter(editor.document());
    
    editor.setWindowTitle("CodeEditor Test");
    editor.resize(800, 600);
    editor.show();

    return app.exec();
}