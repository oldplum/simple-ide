#include "CodeEditor.h"
#include <QPainter>
#include <QResizeEvent>
#include <QTextBlock>
#include <QKeyEvent>
#include <QMessageBox>
#include <QRegularExpression>

// 构造函数，初始化行号栏、高亮连接等
CodeEditor::CodeEditor(QWidget *parent): QPlainTextEdit(parent)  
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

    // 监听文本变化以动态重新计算折叠层级
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::recalculateFolding);
    recalculateFolding();

}

// 当前光标所在行的高亮逻辑（包含整行浅灰背景以及调用matchBracket）
void CodeEditor::highlightCurrentline(){
    QList<QTextEdit::ExtraSelection> selections;

    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(QColor(128,128,128,45));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    selections.append(selection);

    matchBracket(selections); //调用括号匹配算法
    setExtraSelections(selections);
}

// 计算行号栏需要占用的物理宽度（像素），随最大行数变化
int CodeEditor::LineNumberAreaWidth()
{
    int digits = 1;
    int maxLines = qMax(1, blockCount());
    while (maxLines >= 10){
        maxLines /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + 15;
    return space;
}

// 槽函数：当文档的总行数（BlockCount）发生变化时，重新分配左侧行号栏的宽度
void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
}

// 槽函数：当编辑器滚动时（发生垂直偏移 dy），同步滚动和刷新左侧行号栏的绘制区域
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

// 重写：窗口大小改变时，调整内部行号栏区域(lineNumberArea)的几何尺寸 (setGeometry)
void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));
}

// 绘制行号栏的具体逻辑（包括背景色和行号数字）
void CodeEditor::LineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    QColor editorBackground=palette().color(QPalette::Base);
    QColor gutterBackground;
    QColor gutterText;
    if(editorBackground.lightness()<120){
        gutterText=QColor(150,150,150);
        gutterBackground=QColor(37,37,38);
    }
    else{
        gutterText=QColor(90,90,90);
        gutterBackground=QColor(240,240,240);
    }
    painter.fillRect(event->rect(),gutterBackground);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();

    while (block.isValid()){
        // 直接获取当前行在视口中的真实几何矩形
        QRectF blockRect = blockBoundingGeometry(block).translated(contentOffset());
        int top = qRound(blockRect.top());
        int bottom = qRound(blockRect.bottom());

        // 如果当前行已经超出了可见区域底部，直接结束循环
        if (top > event->rect().bottom())
            break;

        // 只有当行真正可见，且在绘制区域内时才绘制
        if (block.isVisible() && bottom >= event->rect().top()){
            QString number = QString::number(blockNumber + 1);
            painter.setPen(gutterText);
            
            // 行号文字偏左，留出右侧 15px 空间画折叠小方块
            int widthForNumber = lineNumberArea->width() - 15;
            painter.drawText(0, top, widthForNumber, fontMetrics().height(), Qt::AlignRight, number);
            
            // 绘制折叠指示器 [+] 或 [-]
            FoldingUserData *data = dynamic_cast<FoldingUserData*>(block.userData());
            if (data && data->isFoldStart()){
                QRect rect(lineNumberArea->width() - 13, top + (fontMetrics().height() - 10) / 2, 10, 10);
                painter.save();
                painter.setPen(gutterText);
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(rect);
                // 绘制一条横线表示减号
                painter.drawLine(rect.left() + 2, rect.top() + 5, rect.right() - 2, rect.top() + 5);
                if (data->isFolded()){
                    // 如果被折叠了，绘制一条竖线组成加号
                    painter.drawLine(rect.left() + 5, rect.top() + 2, rect.left() + 5, rect.bottom() - 2);
                }
                painter.restore();
            }
        }
        
        block = block.next();
        ++blockNumber; // 绝对行号依然正常递增
    }
}


// 重写：拦截键盘输入事件。如果是回车键，则读取上一行的前置空格实现自动缩进；如果是退格键，发射 codeDeleted 信号
void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete)
        emit codeDeleted();

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

// 核心算法：通过栈思想（遇到匹配+1，反向-1）向前或向后搜索匹配的括号对，并将其加入高亮选择区
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

        emit bracketMatched();

    }
}

// 查找下一个/上一个，根据传入的标志组装查找参数
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

    // 如果查到末尾没找到，我们回绕（Wrap-around）从头/尾继续找
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

// 替换当前选中的匹配文本，并查找下一个
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

// 遍历全文，替换所有匹配到的文本
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

// 槽函数：重新计算整个文档中花括号 `{}` 的嵌套层级，用于代码折叠功能
void CodeEditor::recalculateFolding()
{
    QTextBlock block = document()->begin();
    int currentLevel = 0;

    // 使用 QList 模拟栈，记录尚未匹配到 '}' 的折叠起点数据
    QList<FoldingUserData*> openFolds;
    
    while (block.isValid()){
        QString text = block.text();
        bool hasOpenBrace = text.contains('{');
        bool hasCloseBrace = text.contains('}');
        
        FoldingUserData *data = dynamic_cast<FoldingUserData*>(block.userData());
        if (!data){
            data = new FoldingUserData();
            block.setUserData(data);
        }
        
        // 一开始默认当前行不是折叠起点
        data->setFoldStart(false); 
        data->setFoldingLevel(currentLevel);

        // 处理左括号
        if (hasOpenBrace){
            currentLevel++;
            if (!hasCloseBrace)
                openFolds.push_back(data); // 如果当前行只有 '{'，先把它推入栈中等待匹配
        }

        // 处理右括号
        if (hasCloseBrace){
            currentLevel = qMax(0, currentLevel - 1);
            if (!hasOpenBrace){
                // 如果遇到了“单身” '}'，且栈里有等待的 '{'
                if (!openFolds.isEmpty()){
                    // 匹配成功！把最近的一个 '{' 设置为真正的折叠起点
                    FoldingUserData *matchedStart = openFolds.takeLast();
                    matchedStart->setFoldStart(true);
                }
            }
        }
        block = block.next();
    }
}

// 执行折叠或展开动作的核心函数，根据当前块的折叠状态（FoldingUserData），隐藏或显示其子级代码块
void CodeEditor::toggleFold(QTextBlock &startBlock)
{
    FoldingUserData *startData = dynamic_cast<FoldingUserData*>(startBlock.userData());
    if (!startData || !startData->isFoldStart()) 
        return;
    
    startData->setFolded(!startData->isFolded());
    updateFoldedBlocksVisibility();
}

// 遍历所有文本块（QTextBlock），根据它们的 m_isFolded 属性，动态刷新它们在编辑器中的可见性（setVisible）
void CodeEditor::updateFoldedBlocksVisibility()
{
    QTextBlock block = document()->begin();
    int hideLevel = -1; // -1 表示可见，否则表示隐藏该嵌套级别之下的行
    
    while (block.isValid()){
        FoldingUserData *data = dynamic_cast<FoldingUserData*>(block.userData());
        
        if (hideLevel != -1){
            // 如果遇到了包含 '}' 且层级刚好退回来的闭合行
            if (data && data->foldingLevel() == hideLevel + 1 && block.text().contains('}')) {
                block.setVisible(true); // 闭合行保持可见！
                hideLevel = -1;        // 在这里结束隐藏状态
            } 
            else {
                block.setVisible(false);
            }
        } 
        else{
            block.setVisible(true);
            if (data && data->isFoldStart() && data->isFolded())
                hideLevel = data->foldingLevel();
        }
        block = block.next();
    }
    
    emit document()->documentLayout()->update();
    viewport()->update();
    lineNumberArea->update();
}

// 处理在行号栏区域点击鼠标的事件（例如点击折叠代码块）
void CodeEditor::lineNumberAreaMousePressEvent(QMouseEvent *event)
{
    // 如果点击的是行号栏最右侧 15px 以内的折叠按钮区
    if (event->x() >= lineNumberArea->width() - 15){
        int y = event->y();
        QTextBlock block = firstVisibleBlock();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());
        
        while (block.isValid() && top <= lineNumberArea->height()){
            if (block.isVisible() && y >= top && y <= bottom){
                toggleFold(block);
                break;
            }
            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
        }
    }
}
//按ctrl+鼠标滚轮改变字体大小
void CodeEditor::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        const int delta = event->angleDelta().y();
        QFont editorFont =font();
        int fontSize=editorFont.pointSize();
        if (fontSize <= 0){
            fontSize = 11;
        }
        const int minFontSize = 8;
        const int maxFontSize = 32;
        if (delta > 0&&fontSize<maxFontSize) {
            ++fontSize;
        }
        else if (delta < 0&&fontSize>minFontSize){
            --fontSize;
        }
        else{
            event->accept();
                 return;
    }
        editorFont.setPointSize(fontSize);
        setFont(editorFont);

        updateLineNumberAreaWidth(0);
        lineNumberArea->update();

        event->accept();
        return;
    }

    QPlainTextEdit::wheelEvent(event);
}
