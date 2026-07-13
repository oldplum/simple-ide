#include "Highlighter.h"

Highlighter::Highlighter(QTextDocument *parent, const QString &extension)
    : QSyntaxHighlighter(parent)
{
    // 在这里初始化所有高亮规则，往 m_rules 里塞 HighlightRule

    QString ext = extension.toLower();

    // 分支1：如果是JSON文件
    if (ext == "json"){
        HighlightRule rule;

        // 1. 先把所有带引号的都当成 字符串值 -> 深红色
        rule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
        rule.format.setForeground(QColor(180, 0, 0));
        rule.format.setFontWeight(QFont::Normal);
        m_rules.append(rule);

        // 2. 然后再找那些带有冒号的，把它们单独覆盖成 JSON 键名 -> 卡布应援色加粗
        rule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"\\s*(?=:)");
        rule.format.setForeground(QColor(34, 66, 148)); 
        rule.format.setFontWeight(QFont::Bold);
        m_rules.append(rule);

        // 3. 数字 (负号、小数等) -> 紫色
        rule.pattern = QRegularExpression("\\b-?[0-9]+\\.?[0-9]*\\b");
        rule.format.setForeground(QColor(160, 0, 160));
        m_rules.append(rule);

        // 4. 布尔值和 null -> 深青色
        rule.pattern = QRegularExpression("\\b(true|false|null)\\b");
        rule.format.setForeground(QColor(0, 128, 128));
        m_rules.append(rule);

        // JSON 没有多行注释，正则置空
        commentStartExpression = QRegularExpression();
        commentEndExpression = QRegularExpression();
    }

    // 分支2：如果是 C++ / Java / C 等其他语言
    else {
        // 关键字（周深应援色，加粗）
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

        //多行注释格式初始化
        multiLineCommentFormat.setForeground(QColor(128, 128, 128));
        multiLineCommentFormat.setFontItalic(true);
        // 告诉程序 C++ 的多行注释符号是什么
        commentStartExpression = QRegularExpression("/\\*");
        commentEndExpression = QRegularExpression("\\*/");
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    // 1.遍历所有单行高亮规则
    for (const HighlightRule &rule : m_rules){
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()){
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    // 如果当前语言没有多行注释（比如 JSON），直接结束
    if (commentStartExpression.pattern().isEmpty())
        return;
    // 2.处理多行注释
    setCurrentBlockState(0);
    // 把下面所有原来用到 commentStart / commentEnd 的地方，都加上 Expression 后缀！
    int startIndex = 0;
    // 如果上一行已经是多行注释状态（状态为 1），那么本行从开头（0）就开始算作注释
    if (previousBlockState() != 1) {
        QRegularExpressionMatch startMatch = commentStartExpression.match(text);
        startIndex = startMatch.hasMatch() ? startMatch.capturedStart() : -1;
    }
    // 循环寻找注释的结束位置 */
    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = commentEndExpression.match(text, startIndex);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
        int commentLength;
        if (endIndex == -1) {
            // 如果本行没有找到结束符 */，说明注释一直延续到行尾，并通知下一行继续
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            // 找到了结束符 */，计算这一段注释的长度
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        // 涂上灰色斜体
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        // 继续在当前行剩下的地方找有没有下一个 /* 
        QRegularExpressionMatch nextStart = commentStartExpression.match(text, startIndex + commentLength);
        startIndex = nextStart.hasMatch() ? nextStart.capturedStart() : -1;
    }
}