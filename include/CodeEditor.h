#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QTextBlockUserData>
#include <QMouseEvent>

class CodeEditor: public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    int LineNumberAreaWidth();                              //计算行号栏长度
    void LineNumberAreaPaintEvent(QPaintEvent *event);      //绘制行号栏
    void lineNumberAreaMousePressEvent(QMouseEvent *event); //处理行号栏点击事件

public slots:
    void findNext(const QString &text, bool caseSensitive, bool wholeWord, bool useRegex, bool backward);
    void replace(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);
    void replaceAll(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);

private slots:
    void highlightCurrentline();
    void updateLineNumberAreaWidth(int newBlockCount);    // 行数变化时更新宽度
    void updateLineNumberArea(const QRect &rect, int dy); // 滚动时同步行号
    void recalculateFolding();                            // 重新计算折叠嵌套层级


protected:
    void resizeEvent(QResizeEvent *event) override; //窗口缩放时调整行号栏
    void keyPressEvent(QKeyEvent *event) override;  //拦截按键事件（实现自动缩进）

private:
    QWidget *lineNumberArea;                 // 行号栏控制指针
    void matchBracket(QList<QTextEdit::ExtraSelection> &selections);
    void toggleFold(QTextBlock &startBlock); // 执行折叠/展开动作
    void updateFoldedBlocksVisibility();     // 动态刷新行的可见性
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
    void mousePressEvent(QMouseEvent *event) override {
        codeEditor->lineNumberAreaMousePressEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

// 附加在每一行 QTextBlock 上的自定义数据，保存折叠状态与缩进层级
class FoldingUserData: public QTextBlockUserData
{
public:
    FoldingUserData(): m_isFoldStart(false), m_isFolded(false), m_foldingLevel(0) {}
    
    bool isFoldStart() const { return m_isFoldStart; }
    void setFoldStart(bool isStart) { m_isFoldStart = isStart; }
    
    bool isFolded() const { return m_isFolded; }
    void setFolded(bool folded) { m_isFolded = folded; }
    
    int foldingLevel() const { return m_foldingLevel; }
    void setFoldingLevel(int level) { m_foldingLevel = level; }
    
private:
    bool m_isFoldStart; // 是否为折叠起点 (含左大括号)
    bool m_isFolded;    // 是否已被折叠
    int m_foldingLevel; // 当前行的嵌套深度 (层级)
};


#endif //CODEEDITOR_H