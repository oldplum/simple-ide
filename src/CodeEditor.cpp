#include "CodeEditor.h"
#include <QPainter>
#include <QResizeEvent>
#include <QTextBlock>
#include <QKeyEvent>
#include <QMessageBox>
#include <QRegularExpression>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    // 创建行号区域
    lineNumberArea = new LineNumberArea(this);

    // 行数变化 → 更新行号栏宽度
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    // 滚动 → 同步行号栏
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);

    updateLineNumberAreaWidth(0);

    connect(this, &QPlainTextEdit::cursorPositionChanged, 
            this, &CodeEditor::highlightCurrentline);
    
    highlightCurrentline();
}

void CodeEditor::highlightCurrentline(){
    QList<QTextEdit::ExtraSelection> selections;

    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(QColor(232, 232, 232));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    selections.append(selection);

    matchBracket(selections); //调用括号匹配算法
    setExtraSelections(selections);
}

// 根据最大行号的位数计算行号栏宽度
int CodeEditor::LineNumberAreaWidth()
{
    int digits = 1;
    int maxLines = qMax(1, blockCount());
    while (maxLines >= 10){
        maxLines /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

// 设置编辑区左边距，为行号栏腾出空间
void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
}

// 滚动时同步更新行号栏
void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy){
        lineNumberArea->scroll(0, dy);
    } else{
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())){
        updateLineNumberAreaWidth(0);
    }
}

// 窗口大小变化时，调整行号栏的几何尺寸
void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));
}

// 绘制行号
void CodeEditor::LineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()){
        if (block.isVisible() && bottom >= event->rect().top()){
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

//实现回车时自动缩进
void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    // 1. 如果按下的是回车键
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter){
        // 获取光标所在行的文本
        QTextCursor cursor = textCursor();
        QString lineText = cursor.block().text();

        // 提取这行开头的缩进（把空格和 Tab 收集起来）
        QString indentation;
        for (int i = 0; i < lineText.length(); ++i){
            if (lineText.at(i).isSpace()){
                indentation += lineText.at(i);
            } else{
                break;
            }
        }

        // 先让底层的编辑器完成“真正的换行”操作
        QPlainTextEdit::keyPressEvent(e);

        // 然后在刚刚换下来的新行里，填入和上一行一模一样的空格
        if (!indentation.isEmpty()){
            insertPlainText(indentation);
        }
        return;
    }

    // 2. 如果是其他按键，正常处理
    QPlainTextEdit::keyPressEvent(e);
}

// 括号匹配算法实现
void CodeEditor::matchBracket(QList<QTextEdit::ExtraSelection> &selections)
{
    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    QTextDocument *doc = document();
    if (pos == 0) 
        return;

    // 获取光标左边和右边的字符
    QChar leftChar = doc->characterAt(pos - 1);
    QChar rightChar = doc->characterAt(pos);

    // 定义需要匹配的括号对
    QString leftBrackets = "({[";
    QString rightBrackets = ")}]";

    int matchPos = -1;
    int bracketPos = -1;

    // 1. 如果光标左侧或右侧是 左括号 '(' '{' '['
    if (leftBrackets.contains(rightChar) || leftBrackets.contains(leftChar)){
        // 确定到底是哪一边匹配到了左括号
        bracketPos = leftBrackets.contains(rightChar) ? pos : pos - 1;
        QChar left = doc->characterAt(bracketPos);
        QChar right = rightBrackets.at(leftBrackets.indexOf(left));
        int count = 1;
        // 往后找对应的右括号
        for (int i = bracketPos + 1; i < doc->characterCount(); ++i){
            QChar c = doc->characterAt(i);
            if (c == left) 
                count++;
            else if (c == right) 
                count--;
            if (count == 0){
                matchPos = i;
                break;
            }
        }
    } 
    // 2. 如果光标左侧或右侧是 右括号 ')' '}' ']'
    else if (rightBrackets.contains(leftChar) || rightBrackets.contains(rightChar)){
        // 确定到底是哪一边匹配到了右括号
        bracketPos = rightBrackets.contains(leftChar) ? pos - 1 : pos;
        QChar right = doc->characterAt(bracketPos);
        QChar left = leftBrackets.at(rightBrackets.indexOf(right));
        int count = 1;
        // 往前找对应的左括号
        for (int i = bracketPos - 1; i >= 0; --i){
            QChar c = doc->characterAt(i);
            if (c == right) 
                count++;
            else if (c == left) 
                count--;
            if (count == 0){
                matchPos = i;
                break;
            }
        }
    }

    // 3. 如果找到了配对的括号，给它们上色
    if (matchPos != -1){
        QTextCharFormat format;
        format.setBackground(QColor(0, 0, 0, 50));
        // format.setForeground(Qt::red); 

        QTextEdit::ExtraSelection sel1, sel2;
        sel1.format = format;
        sel2.format = format;

        // 选中第一个括号
        QTextCursor c1 = textCursor();
        c1.setPosition(bracketPos);
        c1.setPosition(bracketPos + 1, QTextCursor::KeepAnchor);
        sel1.cursor = c1;

        // 选中配对的第二个括号
        QTextCursor c2 = textCursor();
        c2.setPosition(matchPos);
        c2.setPosition(matchPos + 1, QTextCursor::KeepAnchor);
        sel2.cursor = c2;

        selections.append(sel1);
        selections.append(sel2);
    }
}

// 1. 查找下一个/上一个
void CodeEditor::findNext(const QString &text, bool caseSensitive, bool wholeWord, bool useRegex, bool backward)
{
    QTextDocument::FindFlags flags;
    if (backward)
        flags |= QTextDocument::FindBackward;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    if (wholeWord)
        flags |= QTextDocument::FindWholeWords;

    bool found = false;
    if (useRegex){
        QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
        if (!caseSensitive) 
            options |= QRegularExpression::CaseInsensitiveOption;
        QRegularExpression regex(text, options);
        found = find(regex, flags);
    }
    else
        found = find(text, flags);

    // 体验优化：如果查到末尾没找到，我们回绕（Wrap-around）从头/尾继续找
    if (!found){
        QTextCursor cursor = textCursor();
        int savedPos = cursor.position();

        // 移到文档开头（向下找）或文档末尾（向上找）
        cursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        setTextCursor(cursor);

        // 重新查找一次
        if (useRegex){
            QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
            if (!caseSensitive)
                options |= QRegularExpression::CaseInsensitiveOption;
            QRegularExpression regex(text, options);
            found = find(regex, flags);
        } 
        else
            found = find(text, flags);

        // 如果真的不存在，还原光标位置，并弹窗提示
        if (!found){
            cursor.setPosition(savedPos);
            setTextCursor(cursor);
            QMessageBox::information(this, tr("查找"), tr("找不到目标内容。"));
        }
    }
}

// 2. 替换当前选中项（并自动跳到下一个）
void CodeEditor::replace(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex)
{
    QTextCursor cursor = textCursor();
    
    // 判断当前光标选中的文本，是否就是我们要查找的内容
    if (cursor.hasSelection()){
        QString selected = cursor.selectedText();
        bool match = false;
        
        if (useRegex){
            QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
            if (!caseSensitive)
                options |= QRegularExpression::CaseInsensitiveOption;
            QString pattern = text;
            if (wholeWord)
                pattern = "\\b" + pattern + "\\b";
            QRegularExpression regex("^" + pattern + "$", options);
            match = regex.match(selected).hasMatch();
        } 
        else{
            Qt::CaseSensitivity cs = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
            match = (selected.compare(text, cs) == 0);
            
            if (match && wholeWord){
                // 如果是全词匹配，要确保它有单词边界
                QRegularExpression regex("\\b" + QRegularExpression::escape(text) + "\\b", 
                    caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
                match = regex.match(selected).hasMatch();
            }
        }

        // 如果光标处选中的词刚好匹配，直接替换
        if (match){
            cursor.insertText(replaceText);
        }
    }
    
    // 替换后自动查找下一个
    findNext(text, caseSensitive, wholeWord, useRegex, false);
}

// 3. 全部替换
void CodeEditor::replaceAll(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex)
{
    QTextCursor originalCursor = textCursor();
    
    // 将光标瞬间定位到文档开头
    QTextCursor searchCursor(document());
    searchCursor.movePosition(QTextCursor::Start);
    setTextCursor(searchCursor);

    int count = 0;
    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    if (wholeWord)
        flags |= QTextDocument::FindWholeWords;

    bool found = false;
    do{
        if (useRegex){
            QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
            if (!caseSensitive)
                options |= QRegularExpression::CaseInsensitiveOption;
            QRegularExpression regex(text, options);
            found = find(regex, flags);
        } 
        else
            found = find(text, flags);

        if (found){
            textCursor().insertText(replaceText);
            count++;
        }
    }while (found);

    // 替换完成后，将光标还原回用户之前在的位置，并弹窗汇报战果
    setTextCursor(originalCursor);
    QMessageBox::information(this, tr("全部替换"), tr("替换完成！共替换了 %1 处。").arg(count));
}