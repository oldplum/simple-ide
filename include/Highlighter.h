#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>


class Highlighter: public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent = nullptr, const QString &extension = QString());

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    
    void loadRulesFromFile(const QString &ext); 
    
    QVector<HighlightRule> m_rules;

    QTextCharFormat multiLineCommentFormat;     
    QRegularExpression commentStartExpression;  
    QRegularExpression commentEndExpression;    
};

#endif 