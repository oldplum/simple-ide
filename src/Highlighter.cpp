#include "Highlighter.h"

Highlighter::Highlighter(QTextDocument *parent): QSyntaxHighlighter(parent)
{
    // 在这里初始化所有高亮规则，往 m_rules 里塞 HighlightRule

    
    //关键字（周深应援色，加粗）
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(QColor(34, 66, 148));
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywords;
    keywords << "\\bauto\\b"      << "\\bbreak\\b"    << "\\bcase\\b"
             << "\\bcatch\\b"     << "\\bclass\\b"    << "\\bconst\\b"
             << "\\bconstexpr\\b" << "\\bcontinue\\b" << "\\bdefault\\b"
             << "\\bdelete\\b"    << "\\bdo\\b"       << "\\belse\\b"
             << "\\benum\\b"      << "\\bexplicit\\b" << "\\bextern\\b"
             << "\\bfor\\b"       << "\\bfriend\\b"   << "\\bgoto\\b"
             << "\\bif\\b"        << "\\binline\\b"   << "\\bnamespace\\b"
             << "\\bnew\\b"       << "\\bnoexcept\\b" << "\\bnullptr\\b"
             << "\\boperator\\b"  << "\\boverride\\b" << "\\bprivate\\b"
             << "\\bprotected\\b" << "\\bpublic\\b"   << "\\breturn\\b"
             << "\\bsizeof\\b"    << "\\bstatic\\b"   << "\\bstruct\\b"
             << "\\bswitch\\b"    << "\\btemplate\\b" << "\\bthis\\b"
             << "\\bthrow\\b"     << "\\btry\\b"      << "\\btypedef\\b"
             << "\\btypename\\b"  << "\\bunion\\b"    << "\\busing\\b"
             << "\\bvirtual\\b"   << "\\bvolatile\\b" << "\\bwhile\\b";
    
    for (const QString &kw : keywords) {
        HighlightRule rule;
        rule.pattern = QRegularExpression(kw);
        rule.format = keywordFormat;
        m_rules.append(rule);
    }

    //数据类型（深青色）
    QTextCharFormat typeFormat;
    typeFormat.setForeground(QColor(0, 128, 128));

    QStringList types;
    types << "\\bbool\\b"  << "\\bchar\\b"   << "\\bdouble\\b"
          << "\\bfloat\\b" << "\\bint\\b"    << "\\blong\\b"
          << "\\bshort\\b" << "\\bsigned\\b" << "\\bunsigned\\b"
          << "\\bvoid\\b"  << "\\bsize_t\\b" << "\\bstring\\b"
          << "\\btrue\\b"  << "\\bfalse\\b";

    for (const QString &t : types) {
        HighlightRule rule;
        rule.pattern = QRegularExpression(t);
        rule.format = typeFormat;
        m_rules.append(rule);
    }

    //预处理指令（深绿色）
    QTextCharFormat preprocessorFormat;
    preprocessorFormat.setForeground(QColor(0, 128, 0));

    HighlightRule preprocessorRule;
    preprocessorRule.pattern = QRegularExpression("^\\s*#\\s*\\w+");
    preprocessorRule.format = preprocessorFormat;
    m_rules.append(preprocessorRule);

    //数字字面量（深品红色）
    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor(160, 0, 160));

    HighlightRule numberRule;
    numberRule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    numberRule.format = numberFormat;
    m_rules.append(numberRule);

    //函数名（蓝色）
    QTextCharFormat functionFormat;
    functionFormat.setForeground(QColor(0, 100, 200));

    HighlightRule functionRule;
    functionRule.pattern = QRegularExpression("\\b[a-zA-Z_][a-zA-Z0-9_]*\\s*(?=\\()");
    functionRule.format = functionFormat;
    m_rules.append(functionRule);

    //字符串字面量（深红色）
    QTextCharFormat stringFormat;
    stringFormat.setForeground(QColor(180, 0, 0));
    
    HighlightRule doubleQuoteRule;//双引号字符串
    doubleQuoteRule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
    doubleQuoteRule.format = stringFormat;
    m_rules.append(doubleQuoteRule);

    HighlightRule singleQuoteRule;//单引号字符串
    singleQuoteRule.pattern = QRegularExpression("'(?:[^'\\\\]|\\\\.)*'");
    singleQuoteRule.format = stringFormat;
    m_rules.append(singleQuoteRule);

    //单行注释（灰色，斜体）
    QTextCharFormat commentFormat;
    commentFormat.setForeground(QColor(128, 128, 128));
    commentFormat.setFontItalic(true);

    HighlightRule commentRule;
    commentRule.pattern = QRegularExpression("//[^\n]*");
    commentRule.format = commentFormat;
    m_rules.append(commentRule);
    
}

void Highlighter::highlightBlock(const QString &text)
{
    // 遍历所有单行高亮规则
    for (const HighlightRule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}