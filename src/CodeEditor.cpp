#include "CodeEditor.h"
#include <QPainter>
#include <QResizeEvent>
#include <QTextBlock>
#include <QKeyEvent>
#include <QMessageBox>
#include <QRegularExpression>

CodeEditor::CodeEditor(QWidget *parent): QPlainTextEdit(parent)  
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);

    updateLineNumberAreaWidth(0);

    connect(this, &QPlainTextEdit::cursorPositionChanged, 
            this, &CodeEditor::highlightCurrentline);
    
    highlightCurrentline();

    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::recalculateFolding);
    recalculateFolding();

}

void CodeEditor::highlightCurrentline(){
    QList<QTextEdit::ExtraSelection> selections;

    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(QColor(128,128,128,45));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    selections.append(selection);

    matchBracket(selections); 
    setExtraSelections(selections);
}

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

void CodeEditor::updateLineNumberAreaWidth(int )
{
    setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
}

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

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));
}

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
        QRectF blockRect = blockBoundingGeometry(block).translated(contentOffset());
        int top = qRound(blockRect.top());
        int bottom = qRound(blockRect.bottom());

        if (top > event->rect().bottom())
            break;

        if (block.isVisible() && bottom >= event->rect().top()){
            QString number = QString::number(blockNumber + 1);
            painter.setPen(gutterText);
            
            int widthForNumber = lineNumberArea->width() - 15;
            painter.drawText(0, top, widthForNumber, fontMetrics().height(), Qt::AlignRight, number);
            
            FoldingUserData *data = dynamic_cast<FoldingUserData*>(block.userData());
            if (data && data->isFoldStart()){
                QRect rect(lineNumberArea->width() - 13, top + (fontMetrics().height() - 10) / 2, 10, 10);
                painter.save();
                painter.setPen(gutterText);
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(rect);
                painter.drawLine(rect.left() + 2, rect.top() + 5, rect.right() - 2, rect.top() + 5);
                if (data->isFolded()){
                    painter.drawLine(rect.left() + 5, rect.top() + 2, rect.left() + 5, rect.bottom() - 2);
                }
                painter.restore();
            }
        }
        
        block = block.next();
        ++blockNumber; 
    }
}


void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete)
        emit codeDeleted();

    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter){
        QTextCursor cursor = textCursor();
        QString lineText = cursor.block().text();

        QString indentation;
        for (int i = 0; i < lineText.length(); ++i){
            if (lineText.at(i).isSpace()){
                indentation += lineText.at(i);
            } else{
                break;
            }
        }

        QPlainTextEdit::keyPressEvent(e);

        if (!indentation.isEmpty()){
            insertPlainText(indentation);
        }
        return;
    }

    QPlainTextEdit::keyPressEvent(e);
}

void CodeEditor::matchBracket(QList<QTextEdit::ExtraSelection> &selections)
{
    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    QTextDocument *doc = document();
    if (pos == 0) 
        return;

    QChar leftChar = doc->characterAt(pos - 1);
    QChar rightChar = doc->characterAt(pos);

    QString leftBrackets = "({[";
    QString rightBrackets = ")}]";

    int matchPos = -1;
    int bracketPos = -1;

    if (leftBrackets.contains(rightChar) || leftBrackets.contains(leftChar)){
        bracketPos = leftBrackets.contains(rightChar) ? pos : pos - 1;
        QChar left = doc->characterAt(bracketPos);
        QChar right = rightBrackets.at(leftBrackets.indexOf(left));
        int count = 1;
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
    else if (rightBrackets.contains(leftChar) || rightBrackets.contains(rightChar)){
        bracketPos = rightBrackets.contains(leftChar) ? pos - 1 : pos;
        QChar right = doc->characterAt(bracketPos);
        QChar left = leftBrackets.at(rightBrackets.indexOf(right));
        int count = 1;
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

    if (matchPos != -1){
        QTextCharFormat format;
        format.setBackground(QColor(0, 0, 0, 50));

        QTextEdit::ExtraSelection sel1, sel2;
        sel1.format = format;
        sel2.format = format;

        QTextCursor c1 = textCursor();
        c1.setPosition(bracketPos);
        c1.setPosition(bracketPos + 1, QTextCursor::KeepAnchor);
        sel1.cursor = c1;

        QTextCursor c2 = textCursor();
        c2.setPosition(matchPos);
        c2.setPosition(matchPos + 1, QTextCursor::KeepAnchor);
        sel2.cursor = c2;

        selections.append(sel1);
        selections.append(sel2);

        emit bracketMatched();

    }
}

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

    if (!found){
        QTextCursor cursor = textCursor();
        int savedPos = cursor.position();

        cursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        setTextCursor(cursor);

        if (useRegex){
            QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
            if (!caseSensitive)
                options |= QRegularExpression::CaseInsensitiveOption;
            QRegularExpression regex(text, options);
            found = find(regex, flags);
        } 
        else
            found = find(text, flags);

        if (!found){
            cursor.setPosition(savedPos);
            setTextCursor(cursor);
            QMessageBox::information(this, tr("查找"), tr("找不到目标内容。"));
        }
    }
}

void CodeEditor::replace(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex)
{
    QTextCursor cursor = textCursor();
    
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
                QRegularExpression regex("\\b" + QRegularExpression::escape(text) + "\\b", 
                    caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
                match = regex.match(selected).hasMatch();
            }
        }

        if (match){
            cursor.insertText(replaceText);
        }
    }
    
    findNext(text, caseSensitive, wholeWord, useRegex, false);
}

void CodeEditor::replaceAll(const QString &text, const QString &replaceText, bool caseSensitive, bool wholeWord, bool useRegex)
{
    QTextCursor originalCursor = textCursor();
    
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

    setTextCursor(originalCursor);
    QMessageBox::information(this, tr("全部替换"), tr("替换完成！共替换了 %1 处。").arg(count));
}

void CodeEditor::recalculateFolding()
{
    QTextBlock block = document()->begin();
    int currentLevel = 0;

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
        
        data->setFoldStart(false); 
        data->setFoldingLevel(currentLevel);

        if (hasOpenBrace){
            currentLevel++;
            if (!hasCloseBrace)
                openFolds.push_back(data); 
        }

        if (hasCloseBrace){
            currentLevel = qMax(0, currentLevel - 1);
            if (!hasOpenBrace){
                if (!openFolds.isEmpty()){
                    FoldingUserData *matchedStart = openFolds.takeLast();
                    matchedStart->setFoldStart(true);
                }
            }
        }
        block = block.next();
    }
}

void CodeEditor::toggleFold(QTextBlock &startBlock)
{
    FoldingUserData *startData = dynamic_cast<FoldingUserData*>(startBlock.userData());
    if (!startData || !startData->isFoldStart()) 
        return;
    
    startData->setFolded(!startData->isFolded());
    updateFoldedBlocksVisibility();
}

void CodeEditor::updateFoldedBlocksVisibility()
{
    QTextBlock block = document()->begin();
    int hideLevel = -1; 
    
    while (block.isValid()){
        FoldingUserData *data = dynamic_cast<FoldingUserData*>(block.userData());
        
        if (hideLevel != -1){
            if (data && data->foldingLevel() == hideLevel + 1 && block.text().contains('}')) {
                block.setVisible(true); 
                hideLevel = -1;        
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

void CodeEditor::lineNumberAreaMousePressEvent(QMouseEvent *event)
{
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
