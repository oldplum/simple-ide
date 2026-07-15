#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QTextBlockUserData>
#include <QMouseEvent>
#include <QWheelEvent>

//核心代码编辑器组件，继承自QPlainTextEdit
class CodeEditor: public QPlainTextEdit
{
    Q_OBJECT // Qt的元对象宏，只有加了这个宏，才能使用signals和slots机制

public:
    explicit CodeEditor(QWidget *parent = nullptr);         // 构造函数，初始化行号栏、高亮连接等
    int LineNumberAreaWidth();                              // 计算行号栏需要占用的物理宽度（像素），随最大行数变化
    void LineNumberAreaPaintEvent(QPaintEvent *event);      // 绘制行号栏的具体逻辑（包括背景色和行号数字）
    void lineNumberAreaMousePressEvent(QMouseEvent *event); // 处理在行号栏区域点击鼠标的事件（例如点击折叠代码块）

public slots:
    void findNext(const QString &text, bool caseSensitive, bool wholeWord, bool useRegex, bool backward);                // 查找下一个/上一个，根据传入的标志组装查找参数
    void replace(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);    // 替换当前选中的匹配文本，并查找下一个
    void replaceAll(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex); // 遍历全文，替换所有匹配到的文本

private slots:
    void highlightCurrentline();                          // 当前光标所在行的高亮逻辑（包含整行浅灰背景以及调用matchBracket）
    void updateLineNumberAreaWidth(int newBlockCount);    // 槽函数：当文档的总行数（BlockCount）发生变化时，重新分配左侧行号栏的宽度
    void updateLineNumberArea(const QRect &rect, int dy); // 槽函数：当编辑器滚动时（发生垂直偏移 dy），同步滚动和刷新左侧行号栏的绘制区域
    void recalculateFolding();                            // 槽函数：重新计算整个文档中花括号 `{}` 的嵌套层级，用于代码折叠功能


protected:
    void resizeEvent(QResizeEvent *event) override; //窗口缩放时调整行号栏
    void keyPressEvent(QKeyEvent *event) override;  //拦截按键事件（实现自动缩进）
    void wheelEvent(QWheelEvent *event) override;  //鼠标滚轮改变字体

private:
    QWidget *lineNumberArea;                                         // 指向侧边行号栏控件（LineNumberArea类）的指针，它是依附在当前编辑器上的独立组件
    void matchBracket(QList<QTextEdit::ExtraSelection> &selections); // 核心算法：通过栈思想（遇到匹配+1，反向-1）向前或向后搜索匹配的括号对，并将其加入高亮选择区
    void toggleFold(QTextBlock &startBlock);                         // 执行折叠或展开动作的核心函数，根据当前块的折叠状态（FoldingUserData），隐藏或显示其子级代码块
    void updateFoldedBlocksVisibility();                             // 遍历所有文本块（QTextBlock），根据它们的 m_isFolded 属性，动态刷新它们在编辑器中的可见性（setVisible）

signals:
    void bracketMatched(); // 自定义信号：当编辑器成功匹配到一对括号时发出（用于触发电子猫娘的“开心”状态）
    void codeDeleted();    // 自定义信号：当用户在编辑器中按下退格键或删除键时发出（用于触发电子猫娘的“生气”状态）
};

// 附加组件：行号区域控件
class LineNumberArea: public QWidget
{
public:
    LineNumberArea(CodeEditor *editor): QWidget(editor), codeEditor(editor) {} // 构造函数，保存所属的 CodeEditor 指针，并将自己作为子控件挂载上去
    
    // 重写 sizeHint：告诉 Qt 系统这个行号栏期望占用多宽。
    // 宽度是动态计算的，比如当前代码有 99 行和 1000 行时，需要的宽度不同。
    QSize sizeHint() const override {
        return QSize(codeEditor->LineNumberAreaWidth(), 0);
    }
    
protected:
    // 重写系统绘图事件：当需要更新行号背景或数字时，直接呼叫主编缉器的 Paint 逻辑
    void paintEvent(QPaintEvent *event) override {
        codeEditor->LineNumberAreaPaintEvent(event);
    }
    
    // 重写鼠标点击事件：当用户点击行号区域时（例如为了折叠/展开代码块），转发给主编辑器处理
    void mousePressEvent(QMouseEvent *event) override {
        codeEditor->lineNumberAreaMousePressEvent(event);
    }

private:
    CodeEditor *codeEditor; // 指向宿主主编辑器的指针（用于事件转发）
};

// 附加数据结构：代码块折叠状态信息
class FoldingUserData: public QTextBlockUserData
{
public:
    FoldingUserData(): m_isFoldStart(false), m_isFolded(false), m_foldingLevel(0) {}
    
    bool isFoldStart() const { return m_isFoldStart; }           // 查询该行是否是一个可折叠区域的起点（即含有 '{' ）
    void setFoldStart(bool isStart) { m_isFoldStart = isStart; } // 设置/取消折叠起点标记
    
    bool isFolded() const { return m_isFolded; }                 // 查询该行目前是否处于已被折叠（收缩）的状态
    void setFolded(bool folded) { m_isFolded = folded; }         // 修改当前块的折叠状态标志
    
    int foldingLevel() const { return m_foldingLevel; }          // 查询该行的代码嵌套深度（比如最外层是0，进入一个大括号层级+1）
    void setFoldingLevel(int level) { m_foldingLevel = level; }  // 设置这行的嵌套深度
    
private:
    bool m_isFoldStart; // 核心标志：当前块是否为折叠起点
    bool m_isFolded;    // 核心标志：当前块触发的下级折叠区是否已被收起
    int m_foldingLevel; // 当前行的嵌套深度（层级）
};


#endif //CODEEDITOR_H