#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

class CodeEditor: public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    int LineNumberAreaWidth(); //计算行号栏长度
    void LineNumberAreaPaintEvent(QPaintEvent *event); //绘制行号栏

private slots:
    void highlightCurrentline();
    void updateLineNumberAreaWidth(int newBlockCount);  // 行数变化时更新宽度
    void updateLineNumberArea(const QRect &rect, int dy); // 滚动时同步行号

protected:
    void resizeEvent(QResizeEvent *event) override; //窗口缩放时调整行号栏
    void keyPressEvent(QKeyEvent *event) override;  //拦截按键事件（实现自动缩进）

private:
    QWidget *lineNumberArea; //行号栏控制指针
    void matchBracket(QList<QTextEdit::ExtraSelection> &selections);

};

// 行号区域控件，转发绘制事件给 CodeEditor
class LineNumberArea: public QWidget
{
public:
    LineNumberArea(CodeEditor *editor): QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override {
        return QSize(codeEditor->LineNumberAreaWidth(), 0);
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->LineNumberAreaPaintEvent(event);
    }
private:
    CodeEditor *codeEditor;
};

#endif //CODEEDITOR_H