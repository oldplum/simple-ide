#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QTextBlockUserData>
#include <QMouseEvent>
#include <QWheelEvent>

class CodeEditor: public QPlainTextEdit
{
    Q_OBJECT 

public:
    explicit CodeEditor(QWidget *parent = nullptr);         
    int LineNumberAreaWidth();                              
    void LineNumberAreaPaintEvent(QPaintEvent *event);      
    void lineNumberAreaMousePressEvent(QMouseEvent *event); 

public slots:
    void findNext(const QString &text, bool caseSensitive, bool wholeWord, bool useRegex, bool backward);                
    void replace(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex);    
    void replaceAll(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex); 

private slots:
    void highlightCurrentline();                          
    void updateLineNumberAreaWidth(int newBlockCount);    
    void updateLineNumberArea(const QRect &rect, int dy); 
    void recalculateFolding();                            


protected:
    void resizeEvent(QResizeEvent *event) override; 
    void keyPressEvent(QKeyEvent *event) override;  
    void wheelEvent(QWheelEvent *event) override;  

private:
    QWidget *lineNumberArea;                                         
    void matchBracket(QList<QTextEdit::ExtraSelection> &selections); 
    void toggleFold(QTextBlock &startBlock);                         
    void updateFoldedBlocksVisibility();                             

signals:
    void bracketMatched(); 
    void codeDeleted();    
};

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
    bool m_isFoldStart; 
    bool m_isFolded;    
    int m_foldingLevel; 
};


#endif 
