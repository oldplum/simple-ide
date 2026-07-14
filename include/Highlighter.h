#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>

//语法高亮引擎组件，继承自 QSyntaxHighlighter，这是 Qt 专门用于实现文本语法高亮的核心基类。

class Highlighter: public QSyntaxHighlighter
{
    Q_OBJECT

public:
    // 构造函数：需要传入一个文本文档指针（将其挂载上去），以及文件的扩展名（如 ".cpp"、".py" 用于决定加载哪套规则）
    explicit Highlighter(QTextDocument *parent = nullptr, const QString &extension = QString());

protected:
    // 重写系统函数：Qt 每次需要高亮某一行时，就会自动回调这个函数。参数 text 就是那一行的纯文本内容。
    void highlightBlock(const QString &text) override;

private:
    // ================= 数据结构与配置 =================
    // 定义单个高亮规则的结构体：由一个正则表达式（负责匹配文字）和一个文本格式（负责定义颜色、加粗等样式）组成
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    
    // 从外部 JSON 文件（位于 resources/rules 目录下）加载高亮规则的方法
    void loadRulesFromFile(const QString &ext); 
    
    // 存放所有单行高亮规则（如关键字、字符串、数字）的列表
    QVector<HighlightRule> m_rules;

    // ================= 多行注释状态机 =================
    QTextCharFormat multiLineCommentFormat;     // 多行注释的专属颜色格式
    QRegularExpression commentStartExpression;  // 多行注释的起始标志（例如 "/*" 或 '"""'）
    QRegularExpression commentEndExpression;    // 多行注释的结束标志（例如 "*/" 或 '"""'）
};

#endif //HIGHLIGHTER_H